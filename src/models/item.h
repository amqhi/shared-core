// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef SHARED_ITEM_H
#define SHARED_ITEM_H

#include <string>
#include <optional>
#include "json.hpp"
#include "sqlite3.h"
#include <filesystem>
#include "uuid.h"

struct User;

namespace item_type
{
    constexpr char FILE = 'f';
    constexpr char FOLDER = 'o';
    constexpr char NOTE = 'n';
    constexpr char SONG = 's';
    constexpr char ARTIST = 'a';
    constexpr char ALBUM = 'l';
    constexpr char ALIAS = 'i';
    constexpr char PHOTO = 'p';
    constexpr char UNKNOWN = 'u';

    std::string to_string(char type);
    const char* to_c_str(char type);
    char parse(std::string_view type);
}

namespace app_type
{
    constexpr std::int8_t CLOUD = 1;
    constexpr std::int8_t NOTES = 2;
    constexpr std::int8_t MUSIC = 4;
    constexpr std::int8_t PHOTOS = 8;
    constexpr std::int8_t WEB = 16;
    constexpr std::int8_t AI = 32;
}

namespace icon_type
{
#define DEFINE_ICON_TYPE(name, value, alias_value) constexpr char name = value; \
    constexpr char name##_ALIAS = alias_value;

    DEFINE_ICON_TYPE(FILE, 'f', 'F')
    DEFINE_ICON_TYPE(FOLDER, 'o', 'O')
    DEFINE_ICON_TYPE(STYLED_FOLDER, 'o', 'O')
    DEFINE_ICON_TYPE(TEXT_FILE, 't', 'T')
    DEFINE_ICON_TYPE(IMAGE, 'i', 'I')
    DEFINE_ICON_TYPE(VIDEO, 'v', 'V')
    DEFINE_ICON_TYPE(AUDIO, 'a', 'A')
    DEFINE_ICON_TYPE(MARKDOWN, 'm', 'M')
    DEFINE_ICON_TYPE(PDF, 'p', 'P')
    DEFINE_ICON_TYPE(DOCUMENT, 'd', 'D')
    DEFINE_ICON_TYPE(ARTIST, 'r', 'R')
    DEFINE_ICON_TYPE(ALBUM, 'a', 'A')
    DEFINE_ICON_TYPE(ZIP, 'z', 'Z')

    char parse(int type);
}

struct Item {
    std::string name;
    std::optional<std::string> comment = std::nullopt;
    UUID id;
    UUID parent_id;
    std::int64_t created_at = 0;
    std::int64_t updated_at = 0;
    std::optional<std::int64_t> event_at = std::nullopt;
    std::optional<std::int64_t> deleted_at = std::nullopt;

    /**
       * Bitmask representing which applications can view/access this item.
       * Default value is 63 (0b111111: all scopes enabled).
       */
    std::int16_t app_scope = 63;
    char type;
    char icon_type;
    bool encrypted = false;
    bool cached = false;

    [[nodiscard]] bool is_deleted() const {
        return deleted_at > 0;
    }

    void save(sqlite3* db) const;
};


void item_delete_on_local(const std::string& app_support_path, char user_id, sqlite3* db, const UUID& item_id);

std::filesystem::path item_local_directory_path(const std::string& app_support_path, char user_id, const UUID& id);
std::filesystem::path item_local_file_path(const std::string& app_support_path, char user_id, const UUID& id);
std::filesystem::path item_thumbnail_path(const std::string& app_support_path, char user_id, const UUID& id);

std::int16_t app_scope_from_vector(const std::vector<std::string>& app_scope);
std::int16_t app_scope_from_json(const nlohmann::json& app_scope);
std::vector<std::string> app_scope_to_vector(std::int16_t app_scope);

Item item_from_json(const nlohmann::json& json);

namespace item_column_index
{
    constexpr int ID = 0;
    constexpr int TYPE = 1;
    constexpr int CREATED_AT = 2;
    constexpr int UPDATED_AT = 3;
    constexpr int EVENT_AT = 4;
    constexpr int DELETED_AT = 5;
    constexpr int PARENT_ID = 6;
    constexpr int NAME = 7;
    constexpr int COMMENT = 8;
    constexpr int ICON_TYPE = 9;
    constexpr int ENCRYPTED = 10;
    constexpr int APP_SCOPE = 11;
    constexpr int CACHED = 12;

    namespace bind
    {
        constexpr int ID         = item_column_index::ID + 1;
        constexpr int TYPE       = item_column_index::TYPE + 1;
        constexpr int CREATED_AT = item_column_index::CREATED_AT + 1;
        constexpr int UPDATED_AT = item_column_index::UPDATED_AT + 1;
        constexpr int EVENT_AT   = item_column_index::EVENT_AT + 1;
        constexpr int DELETED_AT = item_column_index::DELETED_AT + 1;
        constexpr int PARENT_ID  = item_column_index::PARENT_ID + 1;
        constexpr int NAME       = item_column_index::NAME + 1;
        constexpr int COMMENT    = item_column_index::COMMENT + 1;
        constexpr int ICON_TYPE = item_column_index::ICON_TYPE + 1;
        constexpr int ENCRYPTED  = item_column_index::ENCRYPTED + 1;
        constexpr int APP_SCOPE  = item_column_index::APP_SCOPE + 1;
        constexpr int CACHED     = item_column_index::CACHED + 1;
    }

}

void sqlite_bind_item(sqlite3_stmt* stmt, const Item& item);
Item item_from_stmt(sqlite3_stmt* stmt);

#endif //SHARED_ITEM_H
