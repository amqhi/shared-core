// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include "common_api.h"

#include <map>

#include "core_base.h"
#include "json_utils.h"
#include "network_error.h"
#include "network_provider_base.h"
#include "uuid.h"
#include "notifier_base.h"
#include "secure_storage_provider_base.h"

void api::request_with_auto_refresh(CoreBase& core, const std::function<void()>& execute_request)
{
    if (core.selected_user()->is_token_refresh_required())
    {
        auth::refresh(core, [execute_request]()
        {
            execute_request();
        });
        return;
    }
    execute_request();
}


void api::sync::get_sync_events_raw(CoreBase& core, const OnResponse& on_response, const OnFailure& on_failure)
{
        std::string url = core.settings().instance_url() + "/sync/events";
        std::map<std::string, std::string> headers;
        headers["Authorization"] = "Bearer " + core.selected_user()->access_token;
        core.network_provider().get(url,
                                  headers,
                                  on_response, on_failure);
}

void api::sync::get_sync_events(CoreBase& core, const OnResponse& on_response, const OnFailure& on_failure)
{
    request_with_auto_refresh(core, [&core, on_response, on_failure]()
    {
        get_sync_events_raw(core, on_response, on_failure);
    });
}

void api::sync::acknowledge_events_raw(
            CoreBase& core,
            const std::vector<std::string>& item_ids,
            const OnResponse& on_response,
            const OnFailure& on_failure)
{
    std::string url = core.settings().instance_url() + "/sync/events/acknowledge";
    std::map<std::string, std::string> headers;
    headers["Authorization"] = "Bearer " + core.selected_user()->access_token;
    nlohmann::json body;
    body["item_ids"] = item_ids;
    core.network_provider().post_json(url, headers, body, on_response, on_failure);
}

void api::sync::acknowledge_events(
            CoreBase& core,
            const std::vector<std::string>& item_ids,
            const OnResponse& on_response,
            const OnFailure& on_failure)
{
    request_with_auto_refresh(core, [&core, item_ids, on_response, on_failure]()
    {
        acknowledge_events_raw(core, item_ids, on_response, on_failure);
    });
}

void api::items::get_item_raw(CoreBase& core,
            const std::string& id, const OnResponse& on_response,
                          const OnFailure& on_failure)
{
    std::string url = core.settings().instance_url() + "/items/" + id;
    std::map<std::string, std::string> headers;
    headers["Authorization"] = "Bearer " + core.selected_user()->access_token;
    core.network_provider().get(url,
                              headers,
                              on_response, on_failure);
}

void api::items::get_item(CoreBase& core,
            const std::string& id, const OnResponse& on_response,
                          const OnFailure& on_failure)
{
    request_with_auto_refresh(core, [&core, id, on_response, on_failure]()
    {
        get_item_raw(core, id, on_response, on_failure);
    });
}

void api::items::get_thumbnail_download_url_raw(
            CoreBase& core,
            const std::string& id,
            const OnResponse& on_response,
            const OnFailure& on_failure)
{
    std::string url = core.settings().instance_url() + "/items/" + id + "/thumbnail/download-url";
    std::map<std::string, std::string> headers;
    headers["Authorization"] = "Bearer " + core.selected_user()->access_token;
    core.network_provider().get(url,
                              headers,
                              on_response, on_failure);
}

void api::items::get_thumbnail_download_url(
            CoreBase& core,
            const std::string& id,
            const OnResponse& on_response,
            const OnFailure& on_failure)
{
    request_with_auto_refresh(core, [&core, id, on_response, on_failure]()
    {
        get_thumbnail_download_url_raw(core, id, on_response, on_failure);
    });
}

void api::files::get_files_raw(CoreBase& core, const OnResponse& on_response,
                           const OnFailure& on_failure)
{
    std::string url = core.settings().instance_url() + "/files";
    std::map<std::string, std::string> headers;
    headers["Authorization"] = "Bearer " + core.selected_user()->access_token;
    core.network_provider().get(url,
                              headers,
                              on_response, on_failure);
}

void api::files::get_files(CoreBase& core, const OnResponse& on_response,
                           const OnFailure& on_failure)
{
    request_with_auto_refresh(core, [&core, on_response, on_failure]()
    {
        get_files_raw(core, on_response, on_failure);
    });
}

void api::folders::get_folders_raw(CoreBase& core, const OnResponse& on_response,
                               const OnFailure& on_failure)
{
    std::string url = core.settings().instance_url() + "/folders";
    std::map<std::string, std::string> headers;
    headers["Authorization"] = "Bearer " + core.selected_user()->access_token;
    core.network_provider().get(url,
                              headers,
                              on_response, on_failure);
}

void api::folders::get_folders(CoreBase& core, const OnResponse& on_response,
                               const OnFailure& on_failure)
{
    request_with_auto_refresh(core, [&core, on_response, on_failure]()
    {
        get_folders_raw(core, on_response, on_failure);
    });
}

void api::files::get_file_metadata_raw(CoreBase& core, const std::string& id, const OnResponse& on_response,
                                   const OnFailure& on_failure)
{
    std::string url = core.settings().instance_url() + "/files/" + id;
    std::map<std::string, std::string> headers;
    headers["Authorization"] = "Bearer " + core.selected_user()->access_token;
    core.network_provider().get(url,
                                headers,
                                on_response,
                                on_failure);
}

void api::files::get_file_metadata(CoreBase& core, const std::string& id, const OnResponse& on_response,
                                   const OnFailure& on_failure)
{
    request_with_auto_refresh(core, [&core, id, on_response, on_failure]()
    {
        get_file_metadata_raw(core, id, on_response, on_failure);
    });
}

void api::files::get_file_metadata(CoreBase& core, const std::string& id, const OnResponse& on_response)
{
    get_file_metadata(core, id, on_response, [&core](std::int16_t error_code, const std::string& data)
    {
        handle_network_error(core.notifier(), error_code, data);
    });
}

void api::files::get_file_metadata(CoreBase& core, const std::string& id,
                                   const std::function<void(FileMetadata& file_metadata)>& on_success)
{
    get_file_metadata(core, id, [on_success](const amqhi::HttpResponse& response)
    {
        if (response.status_code == 200)
        {
            auto body = nlohmann::json::parse(response.body, nullptr, false);
            if (!body.is_discarded())
            {
                FileMetadata file_metadata;
                file_metadata.id = UUID::from_string(json_utils::get_string(body, "id"));
                file_metadata.checksum = json_utils::get_string(body, "checksum");
                file_metadata.size = json_utils::get_int64_t(body, "size");
                file_metadata.mime_type = json_utils::get_string(body, "mime_type");
                on_success(file_metadata);
            }
        }
    });
}

void api::files::get_download_url_raw(CoreBase& core, const std::string& id,
                                  const std::function<void(const amqhi::HttpResponse& response)>& on_response,
                                  const std::function<void(std::int16_t error_code, const std::string& data)>&
                                  on_failure)
{
    std::string url = core.settings().instance_url() + "/files/" + id + "/download-url";
    std::map<std::string, std::string> headers;
    headers["Authorization"] = "Bearer " + core.selected_user()->access_token;
    core.network_provider().get(url,
                                headers,
                                on_response,
                                on_failure);
}

void api::files::get_download_url(CoreBase& core, const std::string& id,
                                  const std::function<void(const amqhi::HttpResponse& response)>& on_response,
                                  const std::function<void(std::int16_t error_code, const std::string& data)>&
                                  on_failure)
{
    request_with_auto_refresh(core, [&core, id, on_response, on_failure]()
    {
        get_download_url_raw(core, id, on_response, on_failure);
    });
}

void api::files::create_file_raw(CoreBase& core, const ItemAttributes& item_attributes, const std::string& mime_type,
                                 std::uint64_t size,
                                 const std::function<void(const amqhi::HttpResponse& response)>& on_response,
                                 const std::function<void(std::int16_t error_code, const std::string& data)>& on_failure)
{
    std::string url = core.settings().instance_url() + "/files";
    std::map<std::string, std::string> headers;
    nlohmann::json body = item_attributes;
    body["mime_type"] = mime_type;
    body["size"] = size;
    headers["Content-Type"] = "application/json";
    headers["Authorization"] = "Bearer " + core.selected_user()->access_token;

    core.network_provider().post_json(url,
                                            headers,
                                            body,
                                            on_response,
                                            on_failure);
}

void api::files::create_file(CoreBase& core, const ItemAttributes& item_attributes, const std::string& mime_type,
                             std::uint64_t size,
                             const std::function<void(const amqhi::HttpResponse& response)>& on_response,
                             const std::function<void(std::int16_t error_code, const std::string& data)>& on_failure)
{
    request_with_auto_refresh(core, [&core, item_attributes, mime_type, size, on_response, on_failure]()
    {
        create_file_raw(core, item_attributes, mime_type, size, on_response, on_failure);
    });
}

void api::files::complete_upload_raw(CoreBase& core, const std::string& item_id,
                                     const std::string& mime_type, const std::string& checksum, std::uint64_t size,
                                     const std::function<void(const amqhi::HttpResponse& response)>& on_response,
                                     const std::function<void(std::int16_t error_code, const std::string& data)>& on_failure)
{
    std::string url = core.settings().instance_url() + "/files/" + item_id + "/complete";
    std::map<std::string, std::string> headers;
    nlohmann::json body;
    headers["Content-Type"] = "application/json";
    headers["Authorization"] = "Bearer " + core.selected_user()->access_token;
    body["checksum"] = checksum;
    body["size"] = size;
    body["mime_type"] = mime_type;
    core.network_provider().post_json(url,
                                        headers,
                                        body,on_response, on_failure);
}

void api::files::complete_upload(CoreBase& core, const std::string& item_id, const std::string& mime_type,
    const std::string& checksum, std::uint64_t size,
    const std::function<void(const amqhi::HttpResponse& response)>& on_response,
    const std::function<void(std::int16_t error_code, const std::string& data)>& on_failure)
{
    request_with_auto_refresh(core, [&core, item_id, mime_type, checksum, size, on_response, on_failure]()
    {
        complete_upload_raw(core, item_id, mime_type, checksum, size, on_response, on_failure);
    });
}

void api::files::complete_multipart_upload_raw(CoreBase& core, const std::string& item_id, const std::string& mime_type,
                                               const std::string& checksum, std::uint64_t size, const nlohmann::json& parts,
                                               const std::string& upload_id,
                                               const std::function<void(const amqhi::HttpResponse& response)>& on_response, const std::function<void(std::int16_t error_code, const std::string& data)>& on_failure)
{
    std::string url = core.settings().instance_url() + "/files/" + item_id + "/complete";
    std::map<std::string, std::string> headers;
    nlohmann::json body;
    headers["Content-Type"] = "application/json";
    headers["Authorization"] = "Bearer " + core.selected_user()->access_token;
    body["checksum"] = checksum;
    body["size"] = size;
    body["mime_type"] = mime_type;
    body["upload_id"] = upload_id;
    body["parts"] = parts;
    core.network_provider().post_json(url,
                                        headers,
                                        body, on_response, on_failure);
}

void api::files::complete_multipart_upload(CoreBase& core, const std::string& item_id, const std::string& mime_type,
    const std::string& checksum, std::uint64_t size, const nlohmann::json& parts, const std::string& upload_id,
    const std::function<void(const amqhi::HttpResponse& response)>& on_response,
    const std::function<void(std::int16_t error_code, const std::string& data)>& on_failure)
{
    request_with_auto_refresh(core, [&core, checksum, item_id, size, mime_type, upload_id, parts, on_response, on_failure]()
    {
        complete_multipart_upload_raw(core, item_id, mime_type, checksum, size, parts, upload_id, on_response, on_failure);
    });
}

void api::items::get_thumbnail_download_url(CoreBase& core, const std::string& id, const OnResponse& on_response)
{
    get_thumbnail_download_url(core, id, on_response, [](std::int16_t error_code, const std::string& data)
    {

    });
}

void api::items::move_items_raw(CoreBase& core, const std::vector<std::string>& ids, const std::string& parent_id, const OnResponse& on_response,
    const OnFailure& on_failure)
{
    std::string url = core.settings().instance_url() + "/items/move";
    std::map<std::string, std::string> headers;
    headers["Authorization"] = "Bearer " + core.selected_user()->access_token;
    nlohmann::json body;
    body["ids"] = ids;
    body["parent_id"] = parent_id;
    core.network_provider().post_json(url,
                                headers,
                                body,
                                on_response,
                                on_failure);

}

void api::items::move_items(CoreBase& core, const std::vector<std::string>& ids, const std::string& parent_id, const OnResponse& on_response,
                            const OnFailure& on_failure)
{
    request_with_auto_refresh(core, [&core, ids, parent_id, on_response, on_failure]()
    {
       move_items_raw(core, ids, parent_id, on_response, on_failure);
    });
}

void api::items::move_items(CoreBase& core, const std::vector<UUID>& ids, const UUID& parent_id,
    const OnResponse& on_response, const OnFailure& on_failure)
{
    std::vector<std::string> string_ids{};
    for (const auto& id : ids) {
        {
            string_ids.push_back(id.to_string());
        }
    }
    move_items(core, string_ids, parent_id.to_string(), on_response, on_failure);
}

void api::items::soft_delete_items_raw(CoreBase& core, const std::vector<std::string>& ids, const OnResponse& on_response,
                                       const OnFailure& on_failure)
{
    std::string url = core.settings().instance_url() + "/items/trash";
    std::map<std::string, std::string> headers;
    headers["Authorization"] = "Bearer " + core.selected_user()->access_token;
    nlohmann::json body;
    body["ids"] = ids;
    core.network_provider().post_json(url,
                                headers,
                                body,
                                on_response,
                                on_failure);
}

void api::items::soft_delete_items(CoreBase& core, const std::vector<std::string>& ids, const OnResponse& on_response,
                                   const OnFailure& on_failure)
{
    request_with_auto_refresh(core, [&core, ids, on_response, on_failure]()
    {
       soft_delete_items_raw(core, ids, on_response, on_failure);
    });
}

void api::items::soft_delete_items(CoreBase& core, const std::vector<UUID>& ids, const OnResponse& on_response,
                                   const OnFailure& on_failure)
{
    std::vector<std::string> string_ids{};
    for (const auto& id : ids)
    {
        string_ids.push_back(id.to_string());
    }
    soft_delete_items(core, string_ids, on_response, on_failure);
}

void api::items::restore_items_raw(CoreBase& core, const std::vector<std::string>& ids, const OnResponse& on_response,
                                   const OnFailure& on_failure)
{
    std::string url = core.settings().instance_url() + "/items/restore";
    std::map<std::string, std::string> headers;
    headers["Authorization"] = "Bearer " + core.selected_user()->access_token;
    nlohmann::json body;
    body["ids"] = ids;
    core.network_provider().post_json(url,
                                headers,
                                body,
                                on_response,
                                on_failure);
}

void api::items::restore_items(CoreBase& core, const std::vector<std::string>& ids, const OnResponse& on_response,
                               const OnFailure& on_failure)
{
    request_with_auto_refresh(core, [&core, ids, on_response, on_failure]()
    {
       restore_items_raw(core, ids, on_response, on_failure);
    });
}

void api::items::restore_items(CoreBase& core, const std::vector<UUID>& ids, const OnResponse& on_response,
                               const OnFailure& on_failure)
{
    std::vector<std::string> string_ids{};
    for (const auto& id : ids)
    {
        string_ids.push_back(id.to_string());
    }
    restore_items(core, string_ids, on_response, on_failure);
}

void api::items::delete_items_raw(CoreBase& core, const std::vector<std::string>& ids, const OnResponse& on_response,
                                  const OnFailure& on_failure)
{
    std::string url = core.settings().instance_url() + "/items/delete";
    std::map<std::string, std::string> headers;
    headers["Authorization"] = "Bearer " + core.selected_user()->access_token;
    nlohmann::json body;
    body["ids"] = ids;
    core.network_provider().post_json(url,
                                headers,
                                body,
                                on_response,
                                on_failure);
}

void api::items::delete_items(CoreBase& core, const std::vector<std::string>& ids, const OnResponse& on_response,
                              const OnFailure& on_failure)
{
    request_with_auto_refresh(core, [&core, ids, on_response, on_failure]()
    {
       delete_items_raw(core, ids, on_response, on_failure);
    });
}

void api::items::delete_items(CoreBase& core, const std::vector<UUID>& ids, const OnResponse& on_response,
                              const OnFailure& on_failure)
{
    std::vector<std::string> string_ids{};
    for (const auto& id : ids)
    {
        string_ids.push_back(id.to_string());
    }
    delete_items(core, string_ids, on_response, on_failure);
}

void api::auth::refresh(CoreBase& core, const std::function<void()>& on_success)
{
    std::string url = core.settings().instance_url() + "/auth/refresh";
    std::map<std::string, std::string> headers;
    nlohmann::json body;
    body["refresh_token"] = core.selected_user()->refresh_token;
    core.network_provider().post_json(
        url,
        headers,
        body,
        [&core, on_success](const amqhi::HttpResponse& response)
        {
            if (response.status_code == 200)
            {
                auto json = nlohmann::json::parse(response.body, nullptr, false);
                if (json.is_discarded())
                {
                    core.selected_user()->access_token = "";
                    core.selected_user()->refresh_token = "";
                    core.notifier().notify(RE_LOGIN_REQUIRED);
                }
                else
                {
                    AuthTokens auth_tokens = AuthTokens::from_json(json);
                    core.selected_user()->access_token = auth_tokens.access_token;
                    core.selected_user()->refresh_token = auth_tokens.refresh_token;
                    core.selected_user()->access_token_expires_at = User::calculate_expires_at(auth_tokens.expires_in);
                    nlohmann::json data;
                   data["access_token"] = auth_tokens.access_token;
                   data["refresh_token"] = auth_tokens.refresh_token;
                   data["expires_at"] = core.selected_user()->access_token_expires_at;

           core.secure_storage_provider().set_secure_string(
               core.selected_user()->local_id,
               data.dump());
                    on_success();
                }
            }
            else
            {
                core.selected_user()->access_token = "";
                core.selected_user()->refresh_token = "";
                core.secure_storage_provider().remove_secure_string(
                    core.selected_user()->local_id);
                core.notifier().notify(RE_LOGIN_REQUIRED);
            }
        }
        , [&core](std::int16_t error_code, const std::string& data)
        {
            handle_network_error(core.notifier(), error_code, data);
        });
}
