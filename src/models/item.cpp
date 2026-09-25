// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include "item.h"

#include "user.h"
#include <string>

#include "date_time_utils.h"
#include "json_utils.h"
#include "sqlite_utils.h"

struct User;

std::string item_type::to_string(char type)
{
    switch (type)
    {
    case NOTE:
        return "note";
    case FOLDER:
        return "folder";
    case SONG:
        return "song";
    case ALBUM:
        return "album";
    case ARTIST:
        return "artist";
    case ALIAS:
        return "alias";
    case PHOTO:
        return "photo";
    default:
        return "unknown";
    }
}

const char* item_type::to_c_str(char type)
{
    switch (type)
    {
    case NOTE:
        return "note";
    case FILE:
        return "file";
    case FOLDER:
        return "folder";
    case SONG:
        return "song";
    case ALBUM:
        return "album";
    case ARTIST:
        return "artist";
    case ALIAS:
        return "alias";
    case PHOTO:
        return "photo";
    default:
        return "unknown";
    }
}

char item_type::parse(std::string_view type)
{
    if (type == "note")
    {
        return NOTE;
    }
    if (type == "file")
    {
        return FILE;
    }
    if (type == "folder")
    {
        return FOLDER;
    }
    if (type == "song")
    {
        return SONG;
    }
    if (type == "album")
    {
        return ALBUM;
    }
    if (type == "artist")
    {
        return ARTIST;
    }
    if (type == "photo")
    {
        return PHOTO;
    }
    return UNKNOWN;
}

void Item::save(sqlite3* db) const
{
    if (db == nullptr)
    {
        return;
    }

    const char* sql =
        "INSERT OR REPLACE INTO items (id, type, created_at, updated_at, event_at, deleted_at, parent_id, name, comment, icon_type, encrypted, app_scope) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);";

    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
    {
        return;
    }

    sqlite_bind_item(stmt, *this);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

void item_delete_on_local(const std::string& app_support_path, char user_local_id, sqlite3* db, const UUID& item_id)
{
    if (db == nullptr)
    {
        return;
    }

    sqlite3_stmt* stmt = nullptr;
    const char* sql = "DELETE FROM items WHERE id = ?;";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
    {
        return;
    }
    sqlite3_bind_blob(stmt, 1, &item_id, sizeof(item_id), SQLITE_TRANSIENT);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    std::filesystem::remove(item_local_directory_path(app_support_path, user_local_id, item_id));
}

std::filesystem::path item_local_directory_path(const std::string& app_support_path, char user_local_id,
                                                const UUID& id)
{
    std::string id_string = id.to_string();
    return std::filesystem::path(app_support_path) / std::string_view(&user_local_id, 1) / "files" /
        std::string(1, id_string.at(0)) / std::string(1, id_string.at(1)) / id_string;
}

std::filesystem::path item_local_file_path(const std::string& app_support_path, char user_local_id, const UUID& id)
{
    std::string id_string = id.to_string();
    return std::filesystem::path(app_support_path) / std::string_view(&user_local_id, 1) / "files" /
        std::string(1, id_string.at(0)) / std::string(1, id_string.at(1)) / id_string / "original";
}

std::filesystem::path item_thumbnail_path(const std::string& app_support_path, char user_local_id, const UUID& id)
{
    std::string id_string = id.to_string();
    return std::filesystem::path(app_support_path) / std::string_view(&user_local_id, 1) / "files" /
        std::string(1, id_string.at(0)) / std::string(1, id_string.at(1)) / id_string / "thumbnail.jpg";
}

std::int16_t app_scope_from_vector(const std::vector<std::string>& data)
{
    std::int16_t app_scope = 0;
    for (const auto& scope : data)
    {
        if (scope == "cloud") app_scope |= app_type::CLOUD;
        else if (scope == "notes") app_scope |= app_type::NOTES;
        else if (scope == "music") app_scope |= app_type::MUSIC;
        else if (scope == "photos") app_scope |= app_type::PHOTOS;
        else if (scope == "web") app_scope |= app_type::WEB;
        else if (scope == "ai") app_scope |= app_type::AI;
    }
    return app_scope;
}

std::int16_t app_scope_from_json(const nlohmann::json& json)
{
    std::int16_t app_scope = 0;
    for (const auto& scope : json)
    {
        if (scope == "cloud") app_scope |= app_type::CLOUD;
        else if (scope == "notes") app_scope |= app_type::NOTES;
        else if (scope == "music") app_scope |= app_type::MUSIC;
        else if (scope == "photos") app_scope |= app_type::PHOTOS;
        else if (scope == "web") app_scope |= app_type::WEB;
        else if (scope == "ai") app_scope |= app_type::AI;
    }
    return app_scope;
}

std::vector<std::string> app_scope_to_vector(std::int16_t app_scope)
{
    std::vector<std::string> scopes;
    scopes.reserve(6);

    if (app_scope & app_type::CLOUD) scopes.emplace_back("cloud");
    if (app_scope & app_type::NOTES) scopes.emplace_back("notes");
    if (app_scope & app_type::MUSIC) scopes.emplace_back("music");
    if (app_scope & app_type::PHOTOS) scopes.emplace_back("photos");
    if (app_scope & app_type::WEB) scopes.emplace_back("web");
    if (app_scope & app_type::AI) scopes.emplace_back("ai");

    return scopes;
}

void sqlite_bind_item(sqlite3_stmt* stmt, const Item& item)
{
    auto id_bytes = item.id.to_bytes();
    sqlite3_bind_blob(stmt, item_column_index::bind::ID, &id_bytes, sizeof(id_bytes), SQLITE_TRANSIENT);

    sqlite3_bind_int(stmt, item_column_index::bind::TYPE, item.type);

    sqlite3_bind_int64(stmt, item_column_index::bind::CREATED_AT, item.created_at);

    sqlite3_bind_int64(stmt, item_column_index::bind::UPDATED_AT, item.updated_at);

    if (item.event_at.has_value())
    {
        sqlite3_bind_int64(stmt, item_column_index::bind::EVENT_AT, item.event_at.value());
    }
    else
    {
        sqlite3_bind_null(stmt, item_column_index::bind::EVENT_AT);
    }

    if (item.deleted_at.has_value())
    {
        sqlite3_bind_int64(stmt, item_column_index::bind::DELETED_AT, item.deleted_at.value());
    }
    else
    {
        sqlite3_bind_null(stmt, item_column_index::bind::DELETED_AT);
    }


    if (item.parent_id != special_folder::HOME && item.parent_id != special_folder::TRASH)
    {
        auto parent_id_bytes = item.parent_id.to_bytes();
        sqlite3_bind_blob(stmt, item_column_index::bind::PARENT_ID, &parent_id_bytes, sizeof(parent_id_bytes),
                          SQLITE_TRANSIENT);
    }
    else
    {
        sqlite3_bind_null(stmt, item_column_index::bind::PARENT_ID);
    }


    sqlite3_bind_text(stmt, item_column_index::bind::NAME, item.name.c_str(), -1, SQLITE_TRANSIENT);

    if (item.comment.has_value())
    {
        sqlite3_bind_text(stmt, item_column_index::bind::COMMENT, item.comment.value().c_str(), -1, SQLITE_TRANSIENT);
    }
    else
    {
        sqlite3_bind_null(stmt, item_column_index::bind::COMMENT);
    }

    sqlite3_bind_int(stmt, item_column_index::bind::ICON_TYPE, item.icon_type);

    sqlite3_bind_int(stmt, item_column_index::bind::ENCRYPTED, item.encrypted ? 1 : 0);

    sqlite3_bind_int(stmt, item_column_index::bind::APP_SCOPE, item.app_scope);
}

Item item_from_stmt(sqlite3_stmt* stmt)
{
    Item item;

    auto get_int64_column = [](sqlite3_stmt* stmt, int col) -> std::optional<long long>
    {
        if (sqlite3_column_type(stmt, col) == SQLITE_NULL)
        {
            return std::nullopt;
        }
        return sqlite3_column_int64(stmt, col);
    };
    const void* id_bytes = sqlite3_column_blob(stmt, item_column_index::ID);
    item.id = UUID::from_bytes(id_bytes);

    item.type = static_cast<char>(sqlite3_column_int(stmt, item_column_index::TYPE));
    item.created_at = sqlite3_column_int64(stmt, item_column_index::CREATED_AT);
    item.updated_at = sqlite3_column_int64(stmt, item_column_index::UPDATED_AT);

    item.event_at = get_int64_column(stmt, item_column_index::EVENT_AT);
    item.deleted_at = get_int64_column(stmt, item_column_index::DELETED_AT);


    if (item.deleted_at.has_value())
    {
        if (sqlite3_column_type(stmt, item_column_index::PARENT_ID) == SQLITE_BLOB)
        {
            const void* parent_id_bytes = sqlite3_column_blob(stmt, item_column_index::PARENT_ID);
            item.parent_id = UUID::from_bytes(parent_id_bytes);
        }
        else
        {
            item.parent_id = special_folder::TRASH;
        }
    }
    else
    {
        if (sqlite3_column_type(stmt, item_column_index::PARENT_ID) == SQLITE_BLOB)
        {
            const void* parent_id_bytes = sqlite3_column_blob(stmt, item_column_index::PARENT_ID);
            item.parent_id = UUID::from_bytes(parent_id_bytes);
        }
        else
        {
            item.parent_id = special_folder::HOME;
        }
    }

    item.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, item_column_index::NAME));
    item.comment = sqlite_utils::get_string(stmt, item_column_index::COMMENT);

    item.encrypted = (sqlite3_column_int(stmt, item_column_index::ENCRYPTED) != 0);
    item.app_scope = static_cast<int16_t>(sqlite3_column_int(stmt, item_column_index::APP_SCOPE));
    item.cached = (sqlite3_column_int(stmt, item_column_index::CACHED) != 0);

    return item;
}

inline std::string parse_nullable_string(const nlohmann::json& json, const std::string& key)
{
    if (json.contains(key) && !json.at(key).is_null())
    {
        if (json.at(key).is_string())
        {
            return json.at(key).get<std::string>();
        }
        else
        {
            return json.at(key).dump();
        }
    }
    return "";
}

Item item_from_json(const nlohmann::json& json)
{
    Item item;

    if (auto it = json.find("id"); it != json.end() && it->is_string())
    {
        item.id = UUID::from_string(it->get<std::string>());
    }
    else
    {
        item.id = special_folder::HOME;
    }
    item.id = UUID::from_string(json_utils::get_string(json, "id"));

    std::string type = json_utils::get_string(json, "type");
    item.type = item_type::parse(type);

    if (auto it = json.find("created_at"); it != json.end() && it->is_string())
    {
        item.created_at = parse_iso8601_to_ms(it->get<std::string>());
    }
    if (auto it = json.find("updated_at"); it != json.end() && it->is_string())
    {
        item.updated_at = parse_iso8601_to_ms(it->get<std::string>());
    }

    if (auto it = json.find("event_at"); it != json.end() && it->is_string())
    {
        item.event_at = parse_iso8601_to_ms(it->get<std::string>());
    }

    if (auto it = json.find("deleted_at"); it != json.end() && it->is_string())
    {
        item.deleted_at = parse_iso8601_to_ms(it->get<std::string>());
    }

    if (auto it = json.find("parent_id"); it != json.end() && it->is_string())
    {
        item.parent_id = UUID::from_string(it->get<std::string>());
    }
    else
    {
        if (item.deleted_at.has_value())
        {
            item.parent_id = special_folder::TRASH;
        }
        else
        {
            item.parent_id = special_folder::HOME;
        }
    }

    item.name = json_utils::get_string(json, "name");

    if (auto it = json.find("comment"); it != json.end() && it->is_string())
    {
        item.comment = parse_nullable_string(json, it->get<std::string>());
    }

    if (json.contains("encrypted") && json["encrypted"].is_boolean())
    {
        item.encrypted = json["encrypted"].get<bool>();
    }
    else
    {
        item.encrypted = false;
    }

    item.app_scope = 0;
    if (json.contains("app_scope") && json["app_scope"].is_array())
    {
        for (const auto& scope : json["app_scope"])
        {
            if (scope == "cloud") item.app_scope |= app_type::CLOUD;
            else if (scope == "notes") item.app_scope |= app_type::NOTES;
            else if (scope == "music") item.app_scope |= app_type::MUSIC;
            else if (scope == "photos") item.app_scope |= app_type::PHOTOS;
            else if (scope == "web") item.app_scope |= app_type::WEB;
            else if (scope == "ai") item.app_scope |= app_type::AI;
        }
    }

    return item;
}
