#include "item_manager_base.h"

#include "common_api.h"
#include "core_base.h"
#include "json_utils.h"
#include "map_utils.h"
#include "notifier_base.h"
#include "request_failure.h"
#include "sqlite_utils.h"
#include <algorithm>
#include <unordered_set>

#include "mime_utils.h"
#include "network_error.h"
#include "network_provider_base.h"
#include "platform_utils_base.h"
#include "item_attributes.h"
#include "date_time_utils.h"
#include "item_utils.h"
#include "sync_event.h"
#include "vector_utils.h"
#include <fstream>

namespace fs = std::filesystem;
using json = nlohmann::json;
using Response = amqhi::HttpResponse;

ItemManagerBase::ItemManagerBase(CoreBase& core) : m_core(core)
{
}

const std::vector<UUID>& ItemManagerBase::id_list_by_id(const UUID& parent_id)
{
    return m_id_lists[parent_id];
}

const Item& ItemManagerBase::item_by_id(const UUID& id)
{
    return m_items[id];
}

const FileMetadata& ItemManagerBase::file_metadata_by_id(const UUID& id)
{
    return m_file_metadata[id];
}

const FolderMetadata& ItemManagerBase::folder_metadata_by_id(const UUID& id)
{
    return m_folder_metadata[id];
}

void ItemManagerBase::clear()
{
    m_items.clear();
    m_id_lists.clear();
    m_file_metadata.clear();
}

void ItemManagerBase::initialize()
{
    Sqlite3Stmt stmt;
    const char* sql = "SELECT * FROM items;";

    if (stmt.prepare(m_core.database_provider().database(), sql) != SQLITE_OK)
    {
        m_core.notifier().notify(DATABASE_ERROR, std::string(sqlite3_errmsg(m_core.database_provider().database())));
        return;
    }

    while (stmt.step() == SQLITE_ROW)
    {
        Item item = item_from_stmt(stmt.stmt);

        m_items[item.id] = item;
        m_id_lists[item.parent_id].push_back(item.id);
    }

    for (const auto& pair : m_id_lists)
    {
        sort_items(pair.first);
    }

    Sqlite3Stmt files_stmt;

    sql = "SELECT * FROM files;";
    files_stmt.prepare(m_core.database_provider().database(), sql);
    while (files_stmt.step() == SQLITE_ROW)
    {
        FileMetadata file_metadata = file_metadata_from_stmt(files_stmt.stmt);

        m_file_metadata[file_metadata.id] = file_metadata;
    }

    Sqlite3Stmt folders_stmt;
    sql = "SELECT * FROM folders;";
    folders_stmt.prepare(m_core.database_provider().database(), sql);
    while (folders_stmt.step() == SQLITE_ROW)
    {
        FolderMetadata folder_metadata = folder_metadata_from_stmt(folders_stmt.stmt);
        auto item_id_bytes = sqlite3_column_blob(folders_stmt.stmt, folder_metadata_column_index::ID);
        UUID item_id = UUID::from_bytes(item_id_bytes);
        m_folder_metadata[item_id] = folder_metadata;
    }
}

void ItemManagerBase::sync()
{
    m_core.notifier().notify(SYNC_STARTED);
    api::sync::get_sync_events(m_core, [this](const Response& response)
                               {
                                   if (response.status_code == 200)
                                   {
                                       nlohmann::json body = nlohmann::json::parse(response.body, nullptr, false);
                                       if (!body.is_discarded())
                                       {
                                           if (auto it = body.find("events"); it != body.end() && it->is_array())
                                           {
                                               const auto& events = it->get<nlohmann::json>();
                                               std::vector<std::string> item_ids;
                                               for (const auto& event : events)
                                               {
                                                   SyncEventType event_type = string_to_sync_event_type(
                                                       json_utils::get_string(event, "type"));
                                                   std::int8_t item_type = item_type::parse(
                                                       json_utils::get_string(event, "item_type"));

                                                   if (auto item_data_raw = event.find("item"); item_data_raw != event.
                                                       end() && item_data_raw->is_object())
                                                   {
                                                       const auto& item_data = item_data_raw->get<nlohmann::json>();
                                                       auto item = item_from_json(item_data);
                                                       UUID item_id = item.id;
                                                       item_ids.emplace_back(item_id.to_string());

                                                       switch (item_type)
                                                       {
                                                       // TODO: Implement remaining item types for item synchronization
                                                       case item_type::FILE:
                                                           {
                                                               auto file_metadata = file_metadata_from_json(item_data);
                                                               cache_file_metadata(
                                                                   m_core.database_provider().database(),
                                                                   file_metadata);
                                                               m_file_metadata[item_id] = file_metadata;
                                                               item.setup_icon_type(file_metadata);
                                                               break;
                                                           }
                                                       case item_type::FOLDER:
                                                           {
                                                               auto folder_metadata = folder_metadata_from_json(
                                                                   item_data);
                                                               cache_folder_metadata(
                                                                   m_core.database_provider().database(), item_id,
                                                                   folder_metadata);
                                                               m_folder_metadata[item_id] = folder_metadata;
                                                               item.setup_icon_type(folder_metadata);
                                                               break;
                                                           }
                                                       default:
                                                           break;
                                                       }

                                                       // TODO: Handle sync for app scope changes
                                                       switch (event_type)
                                                       {
                                                       case SyncEventType::DELETE:
                                                           delete_item(item.id);
                                                           apply_delete_item(item.id, item.parent_id);
                                                           break;
                                                       default:
                                                           item.save(m_core.database_provider().database());
                                                           if (auto pair = m_items.find(item.id); pair != m_items.end())
                                                           // If the item is already exists
                                                           {
                                                               auto& existing_item = pair->second;
                                                               if (existing_item.status ==
                                                                   item_status::AVAILABLE_OFFLINE)
                                                               {
                                                                   const auto old_path = item_local_file_path(
                                                                       m_core, existing_item);

                                                                   if (fs::exists(old_path))
                                                                   {
                                                                       const auto new_path = item_local_file_path(
                                                                           m_core, item);

                                                                       fs::rename(old_path, new_path);
                                                                   }
                                                               }

                                                               if (existing_item.parent_id != item.parent_id || item.
                                                                   deleted_at != existing_item.deleted_at)
                                                               // Handle move or soft-delete or restore
                                                               {
                                                                   UUID old_parent_id = existing_item.parent_id;
                                                                   m_items[item_id] = std::move(item);
                                                                   apply_move_item(
                                                                       item_id, old_parent_id,
                                                                       m_items[item_id].parent_id);
                                                               }
                                                               else // Handle update
                                                               {
                                                                   m_items[item_id] = std::move(item);
                                                                   sort_items(m_items[item_id].parent_id);
                                                               }
                                                           }
                                                           else
                                                           {
                                                               m_id_lists[item.parent_id].push_back(item.id);
                                                               apply_create_item(item);
                                                           }
                                                           break;
                                                       }
                                                   }
                                               }

                                               if (!item_ids.empty())
                                               {
                                                   api::sync::acknowledge_events(m_core, item_ids,
                                                       [this](const Response& response)
                                                       {
                                                           if (response.status_code != 200)
                                                           {
                                                               notify_request_failure(
                                                                   m_core.notifier(),
                                                                   SYNC_EVENTS_ACKNOWLEDGE_FAILURE,
                                                                   response.status_code, response.body);
                                                           }
                                                       }, [this](int error_code,
                                                                 const std::string& data)
                                                       {
                                                           notify_request_failure(
                                                               m_core.notifier(), SYNC_EVENTS_ACKNOWLEDGE_FAILURE,
                                                               error_code, data);
                                                       });
                                               }
                                           }

                                           if (json_utils::get_bool(body, "has_more", false))
                                           {
                                               // TODO: Fully implement sync handling when has_more is true
                                               sync();
                                           }

                                           m_core.notifier().notify(SYNC_SUCCESS);
                                       }
                                       else
                                       {
                                           notify_request_failure(m_core.notifier(), SYNC_EVENTS_FETCH_FAILURE,
                                                                  response.status_code, response.body);
                                       }
                                   }
                                   else
                                   {
                                       notify_request_failure(m_core.notifier(), SYNC_EVENTS_FETCH_FAILURE,
                                                              response.status_code,
                                                              response.body);
                                   }
                               }, [this](std::int16_t error_code, const std::string& data)
                               {
                                   handle_network_error(m_core.notifier(), error_code, data);
                               });
}

// TODO: Handle paginated items during refresh
/// Fetch each item type or thumbnails in order and save it locally. The next step starts after the current request finishes.
void ItemManagerBase::refresh()
{
    refresh_phase1_files();
}

void ItemManagerBase::sort_items(char option, const UUID& parent_id)
{
    m_core.cached_state().set_sort_option(parent_id, option);
    sort_items(parent_id);
}

void ItemManagerBase::sort_items(const UUID& parent_id)
{
    if (m_id_lists.find(parent_id) != m_id_lists.end())
    {
        const auto& sort_options = m_core.cached_state().sort_options();

        if (!map_utils::contains_key(sort_options, parent_id))
        {
            m_core.cached_state().set_sort_option(parent_id, sort_option::DEFAULT_OPTION);
        }

        auto& id_list = m_id_lists[parent_id];

        switch (sort_options.at(parent_id))
        {
        case sort_option::NAME_ASC:
            std::sort(id_list.begin(), id_list.end(),
                      [this](const UUID& a, const UUID& b)
                      {
                          return m_items[a].name < m_items[b].name;
                      });
            break;

        case sort_option::NAME_DESC:
            std::sort(id_list.begin(), id_list.end(),
                      [this](const UUID& a, const UUID& b)
                      {
                          return m_items[a].name > m_items[b].name;
                      });
            break;

        case sort_option::CREATED_AT_ASC:
            std::sort(id_list.begin(), id_list.end(),
                      [this](const UUID& a, const UUID& b)
                      {
                          return m_items[a].created_at < m_items[b].created_at;
                      });
            break;

        case sort_option::CREATED_AT_DESC:
            std::sort(id_list.begin(), id_list.end(),
                      [this](const UUID& a, const UUID& b)
                      {
                          return m_items[a].created_at > m_items[b].created_at;
                      });
            break;

        case sort_option::UPDATED_AT_ASC:
            std::sort(id_list.begin(), id_list.end(),
                      [this](const UUID& a, const UUID& b)
                      {
                          return m_items[a].updated_at < m_items[b].updated_at;
                      });
            break;

        case sort_option::UPDATED_AT_DESC:
            std::sort(id_list.begin(), id_list.end(),
                      [this](const UUID& a, const UUID& b)
                      {
                          return m_items[a].updated_at > m_items[b].updated_at;
                      });
            break;

        case sort_option::SIZE_ASC:
            std::sort(id_list.begin(), id_list.end(),
                      [this](const UUID& a, const UUID& b)
                      {
                          return m_items[a].updated_at < m_items[b].updated_at;
                      });
            break;

        // TODO: sort items by size
        case sort_option::SIZE_DESC:
            std::sort(id_list.begin(), id_list.end(),
                      [this](const UUID& a, const UUID& b)
                      {
                          return m_items[a].updated_at > m_items[b].updated_at;
                      });
            break;

        case sort_option::TYPE_ASC:
            std::sort(id_list.begin(), id_list.end(),
                      [this](const UUID& a, const UUID& b)
                      {
                          return m_items[a].type < m_items[b].type;
                      });
            break;

        case sort_option::TYPE_DESC:
            std::sort(id_list.begin(), id_list.end(),
                      [this](const UUID& a, const UUID& b)
                      {
                          return m_items[a].type > m_items[b].type;
                      });
            break;
        }
    }
}

/// Upload file to server and create metadata
void ItemManagerBase::create_file(const ItemAttributes& item_attributes, const std::string& tmp_file_path)
{
    std::string mime_type = mime_util::get_mime_type_from_file(tmp_file_path);
    std::uint64_t size = std::filesystem::file_size(tmp_file_path);
    std::string checksum = m_core.platform_utils().get_file_sha256_checksum(tmp_file_path);

    api::files::create_file(m_core, item_attributes, mime_type, size, [this, tmp_file_path, checksum, size, mime_type](
                            const Response& response)
                            {
                                if (response.status_code == 201)
                                {
                                    auto body = json::parse(response.body, nullptr, false);
                                    if (!body.is_discarded() && body.contains("item") &&
                                        body["item"].is_object() &&
                                        body.contains("upload") && body["upload"].is_object())
                                    {
                                        Item item = item_from_json(body["item"]);
                                        item.status = item_status::UPLOADING;
                                        auto id = item.id;
                                        m_items[id] = item;
                                        m_id_lists[m_items[id].parent_id].push_back(id);
                                        sort_items(m_items[id].parent_id);
                                        json data;
                                        data["id"] = id.to_string();
                                        data["parent_id"] = m_items[id].parent_id.to_string();
                                        m_core.notifier().notify(FILE_UPLOAD_STARTED, data);

                                        auto& upload = body["upload"];
                                        if (upload["type"] == "multipart")
                                        {
                                            const std::string& upload_id = upload["upload_id"];
                                            auto target_parts = upload.at(
                                                "parts").get<nlohmann::json>();
                                            // Parts allocated by the server

                                            // Payload for multipart completion
                                            auto completed_parts = nlohmann::json::array();

                                            nlohmann::json task_info;
                                            task_info["id"] = item.id.to_string();
                                            task_info["name"] = item.name;
                                            task_info["type"] = transfer_type::MULTIPART_FILE;
                                            task_info["file_size"] = std::to_string(size);
                                            task_info["parent_id"] = item.parent_id.to_string();

                                            m_core.network_provider().put_file(
                                                target_parts,
                                                mime_type,
                                                tmp_file_path,
                                                task_info,
                                                [this, checksum, size, item, mime_type, upload_id,
                                                    completed_parts](
                                                const Response& response)
                                                {
                                                    auto parts_json = json::parse(
                                                        response.body, nullptr, false);
                                                    if (response.status_code == 200)
                                                    {
                                                        complete_upload_multipart(
                                                            item, checksum, size,
                                                            mime_type, upload_id,
                                                            parts_json);
                                                    }
                                                    else
                                                    {
                                                        notify_request_failure(
                                                            m_core.notifier(),
                                                            ITEM_CREATE_FAILURE,
                                                            response.status_code,
                                                            response.body);
                                                    }
                                                }
                                                , [this](std::int16_t error_code,
                                                         const std::string& data)
                                                {
                                                    handle_network_error(
                                                        m_core.notifier(), error_code, data);
                                                });
                                        }
                                        else if (upload["type"] == "single")
                                        {
                                            const std::string& upload_url = upload.at("url").get<
                                                std::string>();
                                            std::map<std::string, std::string> headers;
                                            headers["Content-Type"] = mime_type;
                                            headers["Content-Length"] = std::to_string(size);

                                            nlohmann::json task_info;
                                            task_info["id"] = item.id.to_string();
                                            task_info["name"] = item.name;
                                            task_info["type"] = transfer_type::FILE;
                                            task_info["parent_id"] = item.parent_id.to_string();
                                            m_core.network_provider().put_file(
                                                upload_url,
                                                headers,
                                                tmp_file_path,
                                                task_info,
                                                [this, checksum, size, mime_type, item, upload_url](
                                                const Response& response)
                                                {
                                                    if (response.status_code == 200)
                                                    {
                                                        complete_upload(item, checksum,
                                                                        size, mime_type);
                                                    }
                                                    else
                                                    {
                                                        notify_request_failure(
                                                            m_core.notifier(),
                                                            ITEM_CREATE_FAILURE,
                                                            response.status_code,
                                                            response.body, upload_url);
                                                    }
                                                }
                                                , [this](std::int16_t error_code,
                                                         const std::string& data)
                                                {
                                                    handle_network_error(
                                                        m_core.notifier(), error_code, data);
                                                });
                                        }
                                        else
                                        {
                                            notify_request_failure(m_core.notifier(),
                                                                   ITEM_CREATE_FAILURE,
                                                                   response.status_code,
                                                                   response.body);
                                        }
                                    }
                                    else
                                    {
                                        notify_request_failure(m_core.notifier(),
                                                               ITEM_CREATE_FAILURE,
                                                               response.status_code,
                                                               response.body);
                                    }
                                }
                                else
                                {
                                    notify_request_failure(m_core.notifier(), ITEM_CREATE_FAILURE,
                                                           response.status_code,
                                                           response.body);
                                }
                            },
                            [this](std::int16_t error_code, const std::string& data)
                            {
                                handle_network_error(m_core.notifier(), error_code, data);
                            });
}

void ItemManagerBase::create_folder(const ItemAttributes& item_attributes)
{
    api::request_with_auto_refresh(m_core, [this, &item_attributes]()
    {
        std::string url = m_core.settings().instance_url() + "/folders";
        std::map<std::string, std::string> headers;
        nlohmann::json body = item_attributes;
        headers["Content-Type"] = "application/json";
        headers["Authorization"] = "Bearer " + m_core.selected_user()->access_token;

        m_core.network_provider().post(url,
                                       headers,
                                       body,
                                       [this, url](
                                       const Response& response)
                                       {
                                           if (response.status_code == 201)
                                           {
                                               auto body = json::parse(response.body, nullptr, false);
                                               if (!body.is_discarded())
                                               {
                                                   Item item = item_from_json(body);
                                                   FolderMetadata folder_metadata =
                                                       folder_metadata_from_json(body);
                                                   item.setup_icon_type(folder_metadata);
                                                   item.save(m_core.database_provider().database());
                                                   apply_create_item(item);
                                                   json data;
                                                   data["id"] = item.id.to_string();
                                                   data["parent_id"] = item.parent_id.to_string();
                                                   data["type"] = item.type;
                                                   m_core.notifier().notify(ITEM_CREATE_SUCCESS, data);
                                               }
                                           }
                                           else
                                           {
                                               notify_request_failure(m_core.notifier(), ITEM_CREATE_FAILURE,
                                                                      response.status_code,
                                                                      response.body, url);
                                           }
                                       }, [](std::int16_t error_code, const std::string& data)
                                       {
                                       });
    });
}

void ItemManagerBase::complete_upload_multipart(const Item& item, const std::string& checksum, std::uint64_t size,
                                                const std::string& mime_type, const std::string& upload_id,
                                                const json& parts)
{
    api::files::complete_multipart_upload(m_core,
                                          item.id.to_string(),
                                          mime_type,
                                          checksum,
                                          size,
                                          parts,
                                          upload_id,
                                          [this, item, checksum, mime_type, size](
                                          const Response& response)
                                          {
                                              if (response.status_code == 200)
                                              {
                                                  FileMetadata file_metadata;
                                                  file_metadata.id = item.id;
                                                  file_metadata.checksum = checksum;
                                                  file_metadata.mime_type = mime_type;
                                                  file_metadata.size = size;
                                                  m_file_metadata[item.id] = std::move(file_metadata);
                                                  cache_file_metadata(m_core.database_provider().database(),
                                                                       m_file_metadata[item.id]);
                                                  m_items[item.id] = item;
                                                  m_items[item.id].status = item_status::ONLINE_ONLY;
                                                  m_items[item.id].setup_icon_type( m_file_metadata[item.id]);
                                                  m_items[item.id].save(m_core.database_provider().database());
                                                  json data;
                                                  data["id"] = item.id.to_string();
                                                  data["parent_id"] = item.parent_id.to_string();
                                                  m_core.notifier().notify(ITEM_CREATE_SUCCESS, data);
                                                  download_thumbnail(item.id);
                                              }
                                              else
                                              {
                                                  apply_delete_item(item.id, item.parent_id);
                                                  notify_request_failure(m_core.notifier(), ITEM_CREATE_FAILURE,
                                                                         response.status_code,
                                                                         response.body);
                                              }
                                          }, [this](std::int16_t error_code, const std::string& data)
                                          {
                                              handle_network_error(m_core.notifier(), error_code, data);
                                          });
}

void ItemManagerBase::complete_upload(const Item& item, const std::string& checksum, std::uint64_t size,
                                      const std::string& mime_type)
{
    api::files::complete_upload(m_core,
                                item.id.to_string(),
                                mime_type,
                                checksum,
                                size,
                                [this, item, checksum, mime_type, size](
                                const Response& response)
                                {
                                    if (response.status_code == 200)
                                    {
                                        FileMetadata file_metadata;
                                        file_metadata.id = item.id;
                                        file_metadata.checksum = checksum;
                                        file_metadata.mime_type = mime_type;
                                        file_metadata.size = size;
                                        m_file_metadata[item.id] = std::move(file_metadata);
                                        cache_file_metadata(m_core.database_provider().database(),
                                                             m_file_metadata[item.id]);
                                        m_items[item.id] = item;
                                        m_items[item.id].status = item_status::ONLINE_ONLY;
                                        m_items[item.id].setup_icon_type( m_file_metadata[item.id]);
                                        m_items[item.id].save(m_core.database_provider().database());
                                        json data;
                                        data["id"] = item.id.to_string();
                                        data["parent_id"] = item.parent_id.to_string();
                                        m_core.notifier().notify(ITEM_CREATE_SUCCESS, data);
                                        download_thumbnail(item.id);
                                    }
                                    else
                                    {
                                        apply_delete_item(item.id, item.parent_id);
                                        notify_request_failure(m_core.notifier(), ITEM_CREATE_FAILURE,
                                                               response.status_code,
                                                               response.body);
                                    }
                                }, [this](std::int16_t error_code, const std::string& data)
                                {
                                    handle_network_error(m_core.notifier(), error_code, data);
                                });
}

void ItemManagerBase::update_item(const UUID& id, const ItemAttributes& item_attributes)
{
    api::request_with_auto_refresh(m_core, [this, id, &item_attributes]()
    {
        std::string url = m_core.settings().instance_url() + "/items/" + id.to_string();
        std::map<std::string, std::string> headers;
        headers["Content-Type"] = "application/json";
        headers["Authorization"] = "Bearer " + m_core.selected_user()->access_token;
        nlohmann::json body = item_attributes;

        m_core.network_provider().patch(url,
                                        headers,
                                        body,
                                        [this, id, item_attributes](const Response& response)
                                        {
                                            if (response.status_code == 200)
                                            {
                                                m_items[id].name = item_attributes.name;
                                                m_items[id].parent_id = item_attributes.parent_id;
                                                m_items[id].app_scope = item_attributes.app_scope;
                                                m_items[id].comment = item_attributes.comment;
                                                m_items[id].event_at = item_attributes.event_at;
                                                m_items[id].encrypted = item_attributes.encrypted;
                                                m_items[id].save(m_core.database_provider().database());
                                                json data;
                                                data["id"] = id.to_string();
                                                data["parent_id"] = m_items[id].parent_id.to_string();
                                                sort_items(m_items[id].parent_id);
                                                m_core.notifier().notify(ITEM_UPDATE_SUCCESS, data);
                                            }
                                            else
                                            {
                                                nlohmann::json args;
                                                args["status_code"] = response.status_code;
                                                args["response"] = response.body;
                                                m_core.notifier().notify(ITEM_UPDATE_FAILURE, args);
                                            }
                                        }, [this](std::int16_t error_code, const std::string& data)
                                        {
                                            handle_network_error(m_core.notifier(), error_code, data);
                                        });
    });
}

void ItemManagerBase::move_item(const UUID& id, const UUID& parent_id)
{
    api::request_with_auto_refresh(m_core, [this, id, parent_id]()
    {
        std::string url = m_core.settings().instance_url() + "/items/" + id.to_string() + "/move";
        std::map<std::string, std::string> headers;
        headers["Content-Type"] = "application/json";
        headers["Authorization"] = "Bearer " + m_core.selected_user()->access_token;
        nlohmann::json body;
        if (parent_id != special_folder::TRASH && parent_id != special_folder::HOME)
        {
            body["parent_id"] = parent_id.to_string();
        }
        m_core.network_provider().patch(url,
                                        headers,
                                        body,
                                        [this, id, parent_id](const Response& response)
                                        {
                                            if (response.status_code == 200)
                                            {
                                                UUID old_parent_id = m_items[id].parent_id;
                                                json data;
                                                data["id"] = id.to_string();
                                                data["parent_id"] = parent_id.to_string();
                                                data["old_parent_id"] = old_parent_id.to_string();
                                                m_items[id].parent_id = parent_id;
                                                m_items[id].save(m_core.database_provider().database());
                                                apply_move_item(id, old_parent_id, parent_id);
                                                m_core.notifier().notify(ITEM_MOVE_SUCCESS, data);
                                            }
                                            else
                                            {
                                                nlohmann::json args;
                                                args["status_code"] = response.status_code;
                                                args["response"] = response.body;
                                                m_core.notifier().notify(ITEM_MOVE_FAILURE, args);
                                            }
                                        }, [this](std::int16_t error_code, const std::string& data)
                                        {
                                            handle_network_error(m_core.notifier(), error_code, data);
                                        });
    });
}

void ItemManagerBase::move_items(const std::vector<UUID>& ids, const UUID& parent_id, const UUID& old_parent_id)
{
    vector_utils::subtract<UUID>(m_id_lists[old_parent_id], ids);
    m_id_lists[parent_id].insert(m_id_lists[parent_id].end(), ids.begin(), ids.end());
    sort_items(parent_id);

    m_core.notifier().notify(ITEMS_MOVE_STARTED, old_parent_id.to_string());
    api::items::move_items(m_core, ids, parent_id, [this, ids, parent_id, old_parent_id](const Response& response)
                           {
                               if (response.status_code == 200)
                               {
                                   for (const auto& id : ids)
                                   {
                                       m_items[id].parent_id = parent_id;
                                       m_items[id].save(m_core.database_provider().database());
                                   }
                                   m_core.notifier().notify(ITEMS_MOVE_SUCCESS);
                               }
                               else
                               {
                                   // rollback to previous state
                                   vector_utils::subtract<UUID>(m_id_lists[parent_id], ids);
                                   m_id_lists[old_parent_id].insert(m_id_lists[old_parent_id].end(), ids.begin(),
                                                                    ids.end());
                                   sort_items(old_parent_id);
                                   m_core.notifier().notify(ITEMS_MOVE_FAILURE, response.body);
                               }
                           }, [this, ids, parent_id, old_parent_id](std::int16_t error_code, const std::string& data)
                           {
                               vector_utils::subtract<UUID>(m_id_lists[parent_id], ids);
                               m_id_lists[old_parent_id].
                                   insert(m_id_lists[old_parent_id].end(), ids.begin(), ids.end());
                               sort_items(old_parent_id);
                               m_core.notifier().notify(ITEMS_MOVE_FAILURE);
                           });
}

void ItemManagerBase::rename_item(const UUID& id, const std::string& name)
{
    api::request_with_auto_refresh(m_core, [this, id, &name]()
    {
        std::string url = m_core.settings().instance_url() + "/items/" + id.to_string();
        std::map<std::string, std::string> headers;
        headers["Content-Type"] = "application/json";
        headers["Authorization"] = "Bearer " + m_core.selected_user()->access_token;
        nlohmann::json body;
        body["name"] = name;

        m_core.network_provider().patch(url,
                                        headers,
                                        body,
                                        [this, id, name](const Response& response)
                                        {
                                            if (response.status_code == 200)
                                            {
                                                nlohmann::json data;
                                                m_items[id].name = name;
                                                m_items[id].save(m_core.database_provider().database());
                                                data["id"] = id.to_string();
                                                data["parent_id"] = m_items[id].parent_id.to_string();
                                                m_core.notifier().notify(ITEM_UPDATE_SUCCESS, data);
                                            }
                                            else
                                            {
                                                nlohmann::json args;
                                                args["status_code"] = response.status_code;
                                                args["response"] = response.body;
                                                m_core.notifier().notify(ITEM_UPDATE_FAILURE, args);
                                            }
                                        }, [](std::int16_t error_code, const std::string& data)
                                        {
                                        });
    });
}

void ItemManagerBase::soft_delete_item(const UUID& id)
{
    api::request_with_auto_refresh(m_core, [this, id]()
    {
        std::string url = m_core.settings().instance_url() + "/items/" + id.to_string();
        std::map<std::string, std::string> headers;
        headers["Content-Type"] = "application/json";
        headers["Authorization"] = "Bearer " + m_core.selected_user()->access_token;

        m_core.network_provider().destroy(url,
                                          headers,
                                          [this, id](const Response& response)
                                          {
                                              if (response.status_code == 200)
                                              {
                                                  UUID old_parent_id = m_items[id].parent_id;
                                                  m_items[id].deleted_at = current_date_time_utc_int64();
                                                  m_items[id].parent_id = special_folder::TRASH;
                                                  m_items[id].save(m_core.database_provider().database());
                                                  apply_move_item(id, old_parent_id, special_folder::TRASH);
                                                  nlohmann::json data;
                                                  data["id"] = id.to_string();
                                                  data["parent_id"] = m_items[id].parent_id.to_string();
                                                  data["old_parent_id"] = old_parent_id.to_string();
                                                  m_core.notifier().notify(ITEM_SOFT_DELETE_SUCCESS, data);
                                              }
                                              else
                                              {
                                                  notify_request_failure(m_core.notifier(), ITEM_SOFT_DELETE_FAILURE,
                                                                         response.status_code,
                                                                         response.body);
                                              }
                                          }, [this](std::int16_t error_code, const std::string& data)
                                          {
                                              handle_network_error(m_core.notifier(), error_code, data);
                                          });
    });
}

void ItemManagerBase::soft_delete_items(const std::vector<UUID>& ids, const UUID& old_parent_id)
{
    vector_utils::subtract<UUID>(m_id_lists[old_parent_id], ids);
    m_id_lists[special_folder::TRASH].insert(m_id_lists[special_folder::TRASH].end(), ids.begin(), ids.end());
    sort_items(special_folder::TRASH);

    m_core.notifier().notify(ITEMS_SOFT_DELETE_STARTED, old_parent_id.to_string());
    api::items::soft_delete_items(m_core, ids, [this, ids, old_parent_id](const Response& response)
                                  {
                                      if (response.status_code == 200)
                                      {
                                          for (const auto& id : ids)
                                          {
                                              m_items[id].deleted_at = current_date_time_utc_int64();
                                              m_items[id].parent_id = special_folder::TRASH;
                                              m_items[id].save(m_core.database_provider().database());
                                          }
                                          m_core.notifier().notify(ITEMS_SOFT_DELETE_SUCCESS);
                                      }
                                      else
                                      {
                                          // rollback to previous state
                                          vector_utils::subtract<UUID>(m_id_lists[special_folder::TRASH], ids);
                                          m_id_lists[old_parent_id].insert(
                                              m_id_lists[old_parent_id].end(), ids.begin(), ids.end());
                                          sort_items(old_parent_id);
                                          m_core.notifier().notify(ITEMS_SOFT_DELETE_FAILURE, response.body);
                                      }
                                  }, [this, ids, old_parent_id](std::int16_t error_code, const std::string& data)
                                  {
                                      vector_utils::subtract<UUID>(m_id_lists[special_folder::TRASH], ids);
                                      m_id_lists[old_parent_id].insert(m_id_lists[old_parent_id].end(), ids.begin(),
                                                                       ids.end());
                                      sort_items(old_parent_id);
                                      m_core.notifier().notify(ITEMS_SOFT_DELETE_FAILURE);
                                  });
}

void ItemManagerBase::restore_item(const UUID& id)
{
    api::request_with_auto_refresh(m_core, [this, id]()
    {
        std::string url = m_core.settings().instance_url() + "/items/" + id.to_string() + "/restore";
        std::map<std::string, std::string> headers;
        nlohmann::json body;
        headers["Content-Type"] = "application/json";
        headers["Authorization"] = "Bearer " + m_core.selected_user()->access_token;

        m_core.network_provider().patch(url,
                                        headers,
                                        body,
                                        [this, id](const Response& response)
                                        {
                                            if (response.status_code == 200)
                                            {
                                                UUID old_parent_id = m_items[id].parent_id;
                                                nlohmann::json body = json::parse(response.body, nullptr, false);
                                                m_items[id].deleted_at = std::nullopt;
                                                if (auto it = body.find("parent_id"); it != body.end() && it->
                                                    is_string())
                                                {
                                                    m_items[id].parent_id =
                                                        UUID::from_string(it->get<std::string>());
                                                }
                                                else
                                                {
                                                    m_items[id].parent_id = special_folder::HOME;
                                                }

                                                m_items[id].save(m_core.database_provider().database());
                                                nlohmann::json data;
                                                data["id"] = id.to_string();
                                                data["parent_id"] = m_items[id].parent_id.to_string();
                                                data["old_parent_id"] = old_parent_id.to_string();

                                                apply_move_item(id, old_parent_id, m_items[id].parent_id);
                                                m_core.notifier().notify(ITEM_RESTORE_SUCCESS, data);
                                            }
                                            else
                                            {
                                                std::map<std::string, std::string> args;
                                                args["status_code"] = std::to_string(response.status_code);
                                                args["response"] = response.body;
                                                m_core.notifier().notify(ITEM_RESTORE_FAILURE, args);
                                            }
                                        }, [this](std::int16_t error_code, const std::string& data)
                                        {
                                            handle_network_error(m_core.notifier(), error_code, data);
                                        });
    });
}

void ItemManagerBase::restore_items(const std::vector<UUID>& ids, const UUID& old_parent_id)
{
    vector_utils::subtract<UUID>(m_id_lists[old_parent_id], ids);

    m_core.notifier().notify(ITEMS_RESTORE_STARTED, old_parent_id.to_string());
    api::items::restore_items(m_core, ids, [this, ids, old_parent_id](const Response& response)
                              {
                                  if (response.status_code == 200)
                                  {
                                      json body = json::parse(response.body, nullptr, false);
                                      if (body.is_discarded())
                                      {
                                          m_id_lists[old_parent_id].insert(
                                              m_id_lists[old_parent_id].end(), ids.begin(), ids.end());
                                          sort_items(old_parent_id);
                                          m_core.notifier().notify(ITEMS_RESTORE_FAILURE, response.body);
                                          return;
                                      }
                                      if (!body.is_array())
                                      {
                                          m_id_lists[old_parent_id].insert(
                                              m_id_lists[old_parent_id].end(), ids.begin(), ids.end());
                                          sort_items(old_parent_id);
                                          m_core.notifier().notify(ITEMS_RESTORE_FAILURE, response.body);
                                          return;
                                      }

                                      std::unordered_set<UUID> updated_folder_ids;
                                      for (const auto& element : body)
                                      {
                                          UUID id = UUID::from_string(json_utils::get_string(element, "id"));
                                          UUID original_parent_id = UUID::from_string(
                                              json_utils::get_string(element, "parent_id"));
                                          m_items[id].parent_id = original_parent_id;
                                          m_items[id].save(m_core.database_provider().database());
                                          m_id_lists[original_parent_id].push_back(id);
                                          updated_folder_ids.emplace(original_parent_id);
                                      }

                                      for (const auto& folder_id : updated_folder_ids)
                                      {
                                          sort_items(folder_id);
                                      }

                                      m_core.notifier().notify(ITEMS_RESTORE_SUCCESS);
                                  }
                                  else
                                  {
                                      // rollback to previous state
                                      m_id_lists[old_parent_id].insert(m_id_lists[old_parent_id].end(), ids.begin(),
                                                                       ids.end());
                                      sort_items(old_parent_id);
                                      m_core.notifier().notify(ITEMS_RESTORE_FAILURE, response.body);
                                  }
                              }, [this, ids, old_parent_id](std::int16_t error_code, const std::string& data)
                              {
                                  // rollback to previous state
                                  m_id_lists[old_parent_id].insert(m_id_lists[old_parent_id].end(), ids.begin(),
                                                                   ids.end());
                                  sort_items(old_parent_id);
                                  m_core.notifier().notify(ITEMS_RESTORE_FAILURE);
                              });
}

void ItemManagerBase::delete_item(const UUID& id)
{
    api::request_with_auto_refresh(m_core, [this, id]()
    {
        std::string url = m_core.settings().instance_url() + "/items/" + id.to_string() + "/permanent";
        std::map<std::string, std::string> headers;
        headers["Content-Type"] = "application/json";
        headers["Authorization"] = "Bearer " + m_core.selected_user()->access_token;

        m_core.network_provider().destroy(url,
                                          headers,
                                          [this, id](const Response& response)
                                          {
                                              if (response.status_code == 200)
                                              {
                                                  UUID parent_id = m_items[id].parent_id;
                                                  item_delete_on_local(m_core, m_items[id]);
                                                  if (m_items[id].type == item_type::FILE)
                                                  {
                                                      delete_file_metadata(m_core.database_provider().database(),
                                                                           m_file_metadata[id]);
                                                  }
                                                  apply_delete_item(id, m_items[id].parent_id);
                                                  nlohmann::json data;
                                                  data["id"] = id.to_string();
                                                  data["parent_id"] = parent_id.to_string();

                                                  m_core.notifier().notify(ITEM_DELETE_SUCCESS, data);
                                              }
                                              else
                                              {
                                                  std::map<std::string, std::string> args;
                                                  args["status_code"] = std::to_string(response.status_code);
                                                  args["response"] = response.body;
                                                  m_core.notifier().notify(ITEM_DELETE_FAILURE, args);
                                              }
                                          }, [](std::int16_t error_code, const std::string& data)
                                          {
                                          });
    });
}

void ItemManagerBase::delete_items(const std::vector<UUID>& ids)
{
    vector_utils::subtract<UUID>(m_id_lists[special_folder::TRASH], ids);

    m_core.notifier().notify(ITEMS_DELETE_STARTED);
    api::items::delete_items(m_core, ids, [this, ids](const Response& response)
                                  {
                                      if (response.status_code == 200)
                                      {
                                          for (const auto& id : ids)
                                          {
                                              item_delete_on_local(m_core, m_items[id]);
                                              switch (m_items[id].type)
                                              {
                                              case item_type::FILE:
                                                  delete_file_metadata(m_core.database_provider().database(),
                                                                       m_file_metadata[id]);
                                                  break;
                                              case item_type::FOLDER:
                                                  delete_folder_metadata(m_core.database_provider().database(), id);
                                              default:
                                                  break;
                                              }
                                          }
                                          m_core.notifier().notify(ITEMS_DELETE_SUCCESS);
                                      }
                                      else
                                      {
                                          // rollback to previous state
                                          m_id_lists[special_folder::TRASH].insert(
                                              m_id_lists[special_folder::TRASH].end(), ids.begin(), ids.end());
                                          sort_items(special_folder::TRASH);
                                          m_core.notifier().notify(ITEMS_DELETE_FAILURE, response.body);
                                      }
                                  }, [this, ids](std::int16_t error_code, const std::string& data)
                                  {
                                      m_id_lists[special_folder::TRASH].insert(
                                          m_id_lists[special_folder::TRASH].end(), ids.begin(), ids.end());
                                      sort_items(special_folder::TRASH);
                                      m_core.notifier().notify(ITEMS_DELETE_FAILURE);
                                  });
}

void ItemManagerBase::download_thumbnails(int16_t event_code)
{
    if (m_items.empty())
    {
        m_core.notifier().notify(event_code);
        return;
    }

    int* count = new int(0);
    for (auto& it : m_items)
    {
        const auto& id = it.first;
        if (m_file_metadata[id].mime_type.compare(0, 6, "image/") != 0 && m_file_metadata[id].mime_type.
            compare(0, 6, "video/") != 0 && m_file_metadata[id].mime_type.compare(0, 6, "audio/") != 0)
        {
            (*count)++;
            if (*count == m_items.size())
            {
                delete count;
                m_core.notifier().notify(event_code);
            }
            continue;
        }

        api::items::get_thumbnail_download_url(m_core, id.to_string(),
                                               [this, id, count, event_code](
                                               const Response& response)
                                               {
                                                   fs::path thumbnail_path = item_thumbnail_path(m_core, id);
                                                   if (!fs::exists(thumbnail_path.parent_path()))
                                                   {
                                                       fs::create_directories(thumbnail_path.parent_path());
                                                   }
                                                   if (response.status_code == 200)
                                                   {
                                                       const std::string& download_url = response.body;
                                                       std::map<std::string, std::string> headers;
                                                       nlohmann::json task_info;
                                                       task_info["type"] = transfer_type::THUMBNAIL;
                                                       task_info["id"] = id.to_string();
                                                       task_info["parent_id"] = m_items[id].parent_id.to_string();
                                                       m_core.network_provider().download_file(
                                                           download_url,
                                                           headers,
                                                           thumbnail_path.string(),
                                                           task_info,
                                                           [this, count, event_code](
                                                           const Response& response)
                                                           {
                                                               (*count)++;
                                                               if (*count == m_items.size())
                                                               {
                                                                   delete count;
                                                                   m_core.notifier().notify(event_code);
                                                               }
                                                           }, [this, count, event_code](
                                                           std::int16_t error_code, const std::string& data)
                                                           {
                                                               (*count)++;
                                                               if (*count == m_items.size())
                                                               {
                                                                   delete count;
                                                                   m_core.notifier().notify(event_code);
                                                               }
                                                           });
                                                   }
                                                   else
                                                   {
                                                       (*count)++;
                                                       nlohmann::json data;
                                                       data["status_code"] = response.status_code;
                                                       data["response"] = response.body;
                                                       data["id"] = id.to_string();
                                                       m_core.notifier().notify(ITEM_THUMBNAIL_DOWNLOAD_FAILURE, data);
                                                       if (*count == m_items.size())
                                                       {
                                                           delete count;
                                                           m_core.notifier().notify(event_code);
                                                       }
                                                   }
                                               }, [this, count, event_code](
                                               std::int16_t error_code, const std::string& data)
                                               {
                                                   (*count)++;
                                                   if (*count == m_items.size())
                                                   {
                                                       delete count;
                                                       m_core.notifier().notify(event_code);
                                                   }
                                               });
    }
}

void ItemManagerBase::refresh_phase1_files()
{
    api::request_with_auto_refresh(m_core, [this]()
    {
        api::files::get_files(m_core, [this](const Response& response)
                              {
                                  if (response.status_code == 200)
                                  {
                                      auto files_body = json::parse(response.body, nullptr, false);
                                      if (files_body.is_discarded() || !files_body.is_object())
                                      {
                                          notify_request_failure(m_core.notifier(), REFRESH_FAILURE,
                                                                 response.status_code,
                                                                 response.body);
                                      }
                                      else
                                      {
                                          // bool has_more_files = json_utils::get_bool(files_body, "has_more", false);
                                          // if (has_more_files)
                                          // {
                                          //
                                          // }
                                          if (auto it = files_body.find("files"); it != files_body.end() && it->
                                              is_array())
                                          {
                                              for (auto& child : *it)
                                              {
                                                  Item item = item_from_json(child);
                                                  if (item.updated_at == m_items[item.id].updated_at)
                                                  {
                                                      continue;
                                                  }
                                                  FileMetadata file_metadata = file_metadata_from_json(child);
                                                  item.setup_icon_type(file_metadata);
                                                  const auto item_id = item.id;
                                                  const auto parent_id = item.parent_id;

                                                  item.save(m_core.database_provider().database());
                                                  cache_file_metadata(m_core.database_provider().database(),
                                                                      file_metadata);

                                                  m_items[item_id] = std::move(item);
                                                  m_id_lists[parent_id].push_back(item_id);
                                                  m_file_metadata[item_id] = std::move(file_metadata);
                                              }
                                          }

                                          refresh_phase2_folders();
                                      }
                                  }
                                  else
                                  {
                                      notify_request_failure(m_core.notifier(), FETCH_FILES_FAILURE,
                                                             response.status_code,
                                                             response.body);
                                      refresh_phase2_folders();
                                  }
                              }, [this](std::int16_t error_code, const std::string& data)
                              {
                                  handle_network_error(m_core.notifier(), error_code, data);
                                  refresh_phase2_folders();
                              });
    });
}

void ItemManagerBase::refresh_phase2_folders()
{
    api::request_with_auto_refresh(m_core, [this]()
    {
        api::folders::get_folders(m_core, [this](const Response& response)
                                  {
                                      if (response.status_code == 200)
                                      {
                                          auto folders_body = json::parse(
                                              response.body, nullptr, false);
                                          if (folders_body.is_discarded() || !folders_body.
                                              is_object())
                                          {
                                              notify_request_failure(
                                                  m_core.notifier(),
                                                  REFRESH_FAILURE, response.status_code,
                                                  response.body);
                                          }
                                          else
                                          {
                                              // bool has_more_folders = json_utils::get_bool(folders_body, "has_more", false);
                                              // if (has_more_folders)
                                              // {
                                              //
                                              // }
                                              if (auto it = folders_body.find("folders"); it
                                                  != folders_body.end() && it->is_array())
                                              {
                                                  for (auto& child : *it)
                                                  {
                                                      Item item = item_from_json(child);
                                                      if (item.updated_at == m_items[item.id].updated_at)
                                                      {
                                                          continue;
                                                      }
                                                      FolderMetadata folder_metadata = folder_metadata_from_json(child);
                                                      item.setup_icon_type(folder_metadata);

                                                      const auto item_id = item.id;
                                                      const auto parent_id = item.
                                                          parent_id;

                                                      item.save(
                                                          m_core.database_provider().
                                                                 database());
                                                      cache_folder_metadata(
                                                          m_core.database_provider().database(), item_id,
                                                          folder_metadata);

                                                      m_folder_metadata[item_id] = folder_metadata;
                                                      m_items[item_id] = std::move(item);
                                                      m_id_lists[parent_id].push_back(
                                                          item_id);
                                                  }
                                              }

                                              download_thumbnails(REFRESH_SUCCESS);
                                          }
                                      }
                                      else
                                      {
                                          notify_request_failure(
                                              m_core.notifier(), FETCH_FOLDERS_FAILURE,
                                              response.status_code, response.body);
                                      }
                                  }, [this](std::int16_t error_code,
                                            const std::string& data)
                                  {
                                      handle_network_error(
                                          m_core.notifier(), error_code, data);
                                  });
    });
}

void ItemManagerBase::download_thumbnail(const UUID& id)
{
    api::items::get_thumbnail_download_url(m_core, id.to_string(),
                                           [this, id](const Response& response)
                                           {
                                               fs::path thumbnail_path = item_thumbnail_path(m_core, id);
                                               if (!fs::exists(thumbnail_path.parent_path()))
                                               {
                                                   fs::create_directories(thumbnail_path.parent_path());
                                               }
                                               if (response.status_code == 200)
                                               {
                                                   const std::string& download_url = response.body;
                                                   std::map<std::string, std::string> headers;
                                                   nlohmann::json task_info;
                                                   task_info["type"] = transfer_type::THUMBNAIL;
                                                   task_info["id"] = id.to_string();
                                                   task_info["parent_id"] = m_items[id].parent_id.to_string();
                                                   m_core.network_provider().download_file(
                                                       download_url,
                                                       headers,
                                                       thumbnail_path.string(),
                                                       task_info,
                                                       [this, id, download_url](
                                                       const Response& response)
                                                       {
                                                           if (response.status_code == 200)
                                                           {
                                                               m_core.notifier().
                                                                      notify(ITEM_THUMBNAIL_DOWNLOAD_SUCCESS);
                                                           }
                                                           else
                                                           {
                                                               nlohmann::json data;
                                                               data["status_code"] = response.status_code;
                                                               data["response"] = response.body;
                                                               data["id"] = id.to_string();
                                                               data["url"] = download_url;
                                                               m_core.notifier().notify(
                                                                   ITEM_THUMBNAIL_DOWNLOAD_FAILURE, data);
                                                           }
                                                       }, [](std::int16_t error_code, const std::string& data)
                                                       {
                                                       });
                                               }
                                               else
                                               {
                                                   nlohmann::json data;
                                                   data["status_code"] = response.status_code;
                                                   data["response"] = response.body;
                                                   data["id"] = id.to_string();
                                                   m_core.notifier().notify(ITEM_THUMBNAIL_DOWNLOAD_FAILURE, data);
                                               }
                                           }, [](std::int16_t error_code, const std::string& data)
                                           {
                                           });
}

void ItemManagerBase::make_item_available_offline(const UUID& id)
{
    std::string file_path = item_local_file_path(m_core, m_items[id]).string();
    download_item(id, file_path, [this, id](const Response& response)
    {
        if (response.status_code == 200)
        {
            json data;
            m_items[id].status = item_status::AVAILABLE_OFFLINE;
            m_items[id].save(m_core.database_provider().database());
            data["id"] = id.to_string();
            data["parent_id"] = m_items[id].parent_id.to_string();
            m_core.notifier().notify(ITEM_MAKE_AVAILABLE_OFFLINE_SUCCESS, data);
        }
        else
        {
            notify_request_failure(m_core.notifier(), ITEM_MAKE_AVAILABLE_OFFLINE_FAILURE, response.status_code,
                                   response.body);
        }
    });
}

void ItemManagerBase::download_item(const UUID& id, const std::string& file_path)
{
    download_item(id, file_path, [this, id](const Response& response)
    {
        if (response.status_code == 200)
        {
            nlohmann::json data;
            data["id"] = id.to_string();
            data["parent_id"] = m_items[id].parent_id.to_string();
            m_core.notifier().notify(FILE_DOWNLOAD_SUCCESS, data);
        }
        else
        {
            notify_request_failure(m_core.notifier(), FILE_DOWNLOAD_FAILURE, response.status_code,
                                   response.body);
        }
    });
}

void ItemManagerBase::download_item(const UUID& id, const std::string& file_path,
                                    const std::function<void(const Response& response)>&
                                    on_response)
{
    if (m_items[id].status == item_status::AVAILABLE_OFFLINE)
    {
        if (fs::exists(item_local_file_path(m_core, m_items[id])))
        {
            fs::copy_file(item_local_file_path(m_core, m_items[id]), file_path, fs::copy_options::none);
            Response response;
            response.status_code = 200;
            on_response(response);
            return;
        }
    }
    api::files::get_download_url(
        m_core,
        id.to_string(),
        [this, id, file_path, on_response](const Response& response)
        {
            if (response.status_code == 200)
            {
                const std::string& download_url = response.body;
                std::map<std::string, std::string> headers;
                nlohmann::json task_info;
                task_info["id"] = id.to_string();
                task_info["parent_id"] = m_items[id].parent_id.to_string();
                task_info["type"] = transfer_type::FILE;
                m_core.network_provider().download_file(
                    download_url, headers, file_path,
                    task_info, on_response, [this](std::int16_t error_code, const std::string& data)
                    {
                        handle_network_error(m_core.notifier(), error_code, data);
                    });
            }
            else
            {
                notify_request_failure(m_core.notifier(), FILE_DOWNLOAD_FAILURE, response.status_code,
                                       response.body);
            }
        },
        [this](std::int16_t error_code, const std::string& data)
        {
            handle_network_error(m_core.notifier(), error_code, data);
        });
}

void ItemManagerBase::fetch_file_download_url(const UUID& id) const
{
    api::files::get_download_url(m_core, id.to_string(), [this, id](const Response& response)
                                 {
                                     if (response.status_code == 200)
                                     {
                                         const std::string& download_url = response.body;
                                         json data;
                                         data["id"] = id.to_string();
                                         data["url"] = download_url;
                                         m_core.notifier().notify(FETCH_FILE_DOWNLOAD_URL_SUCCESS, data);
                                     }
                                     else
                                     {
                                         json data;
                                         data["id"] = id.to_string();
                                         m_core.notifier().notify(FETCH_FILE_DOWNLOAD_URL_FAILURE, data);
                                     }
                                 },
                                 [this](std::int16_t error_code, const std::string& data)
                                 {
                                     handle_network_error(m_core.notifier(), error_code, data);
                                 });
}

void ItemManagerBase::apply_create_item(const Item& item)
{
    m_items[item.id] = item;
    m_id_lists[item.parent_id].emplace_back(item.id);
    sort_items(item.parent_id);
}

void ItemManagerBase::apply_update_item(const Item& item)
{
    m_items[item.id] = item;
}

void ItemManagerBase::apply_move_item(const UUID& id, const UUID& old_parent_id, const UUID& parent_id)
{
    m_items[id].parent_id = parent_id;
    if (m_id_lists.find(old_parent_id) != m_id_lists.end())
    {
        auto& old_list = m_id_lists[old_parent_id];
        old_list.erase(std::remove(old_list.begin(), old_list.end(), id),
                       old_list.end());
    }
    if (m_id_lists.find(parent_id) != m_id_lists.end())
    {
        auto& new_list = m_id_lists[parent_id];
        new_list.push_back(id);
    }
    sort_items(m_items[id].parent_id);
}

void ItemManagerBase::apply_delete_item(const UUID& id, const UUID& parent_id)
{
    if (map_utils::contains_key(m_id_lists, parent_id))
    {
        auto& list = m_id_lists[parent_id];
        list.erase(std::remove(list.begin(), list.end(), id),
                   list.end());
        m_items.erase(id);
    }
}
