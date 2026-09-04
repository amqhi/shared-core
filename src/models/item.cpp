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

std::string item_type::to_string(std::int8_t type)
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

const char* item_type::to_c_str(std::int8_t type)
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

std::int8_t item_type::parse(std::string_view type)
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

std::array<std::uint8_t, 16> ItemId::to_bytes() const noexcept
{
    std::array<std::uint8_t, 16> bytes{};
    std::memcpy(bytes.data(), &high, 8);
    std::memcpy(bytes.data() + 8, &low, 8);
    return bytes;
}

ItemId ItemId::from_bytes(const void* blob_data) noexcept
{
    ItemId id{};
    std::memcpy(&id.high, blob_data, 8);
    std::memcpy(&id.low, static_cast<const std::uint8_t*>(blob_data) + 8, 8);
    return id;
}

ItemId ItemId::from_string(std::string_view str) noexcept
{
    if (str.size() != 36 && str.size() != 32) {
        return special_folder::UNKNOWN;
    }

    uint64_t high = 0;
    uint64_t low = 0;
    size_t hex_count = 0;

    for (char c : str) {
        if (c == '-') {
            continue;
        }

        const std::int8_t nibble = hex_char_to_nibble(c);
        if (nibble < 0) {
            return special_folder::UNKNOWN;
        }

        if (hex_count < 16) {
            high = (high << 4) | static_cast<uint64_t>(nibble);
        } else if (hex_count < 32) {
            low = (low << 4) | static_cast<uint64_t>(nibble);
        } else {
            return special_folder::UNKNOWN;
        }

        ++hex_count;
    }

    if (hex_count != 32) {
        return special_folder::UNKNOWN;
    }

    return ItemId{high, low};
}
#if __cplusplus >= 202002L
std::strong_ordering ItemId::operator<=>(const ItemId& other) const
{
    if (auto cmp = this->high <=> other.high; cmp != 0) {
        return cmp;
    }
    return this->low <=> other.low;
}
#else
bool ItemId::operator!=(const ItemId& other) const noexcept
{
    return !(*this == other);
}
bool ItemId::operator<(const ItemId& other) const noexcept
{
    return std::tie(high, low) < std::tie(other.high, other.low);
}

bool ItemId::operator<=(const ItemId& other) const noexcept
{
    return other >= *this;
}

bool ItemId::operator>(const ItemId& other) const noexcept
{
    return other < *this;
}

bool ItemId::operator>=(const ItemId& other) const noexcept
{
    return !(*this < other);
}
#endif

bool ItemId::operator==(const ItemId& other) const
{
    return this->high == other.high && this->low == other.low;
}

[[nodiscard]] std::string ItemId::to_string() const
{
    std::string result(36, '-');

    constexpr char hex_digits[] = "0123456789abcdef";


    size_t char_idx = 0;
    for (int i = 60; i >= 0; i -= 4) {
        if (char_idx == 8 || char_idx == 13) {
            ++char_idx;
        }
        uint8_t nibble = (high >> i) & 0x0F;
        result[char_idx++] = hex_digits[nibble];
    }


    ++char_idx;
    for (int i = 60; i >= 0; i -= 4) {
        if (char_idx == 23) {
            ++char_idx;
        }
        uint8_t nibble = (low >> i) & 0x0F;
        result[char_idx++] = hex_digits[nibble];
    }

    return result;
}

void Item::save(sqlite3* db) const
{
    if (db == nullptr) {
        return;
    }

    const char* sql = "INSERT OR REPLACE INTO items (id, type, created_at, updated_at, event_at, deleted_at, parent_id, name, tags, comment, encrypted, app_scope) "
                      "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);";

    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return;
    }

    sqlite_bind_item(stmt, *this);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);

}

void item_delete_on_local(const std::string& app_support_path, const std::uint8_t user_local_id, sqlite3* db, const ItemId& item_id)
{
    if (db == nullptr) {
        return;
    }
    
    sqlite3_stmt* stmt = nullptr;
    const char* sql = "DELETE FROM items WHERE id = ?;";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return;
    }
    sqlite3_bind_blob(stmt, 1, &item_id, sizeof(item_id), SQLITE_TRANSIENT);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    std::filesystem::remove(item_local_directory_path(app_support_path, user_local_id, item_id));

}

std::filesystem::path item_local_directory_path(const std::string& app_support_path, const std::uint8_t user_local_id, const ItemId& id)
{
    std::string id_string = id.to_string();
    return std::filesystem::path(app_support_path) / std::to_string(user_local_id) / "files" / std::string(1, id_string.at(0)) / std::string(1, id_string.at(1)) / id_string;
}

std::filesystem::path item_local_file_path(const std::string& app_support_path, const std::uint8_t user_local_id, const ItemId& id)
{
    std::string id_string = id.to_string();
    return std::filesystem::path(app_support_path) / std::to_string(user_local_id) / "files" / std::string(1, id_string.at(0)) / std::string(1, id_string.at(1)) / id_string / "original";
}

std::filesystem::path item_thumbnail_path(const std::string& app_support_path, const std::uint8_t user_local_id, const ItemId& id)
{
    std::string id_string = id.to_string();
    return std::filesystem::path(app_support_path) / std::to_string(user_local_id)/ "files" / std::string(1, id_string.at(0)) / std::string(1, id_string.at(1)) / id_string / "thumbnail.jpg";
}

std::int16_t app_scope_from_vector(const std::vector<std::string>& data)
{
    std::int16_t app_scope = 0;
    for (const auto& scope : data) {
        if (scope == "cloud") app_scope |= 1;
        else if (scope == "notes") app_scope |= 2;
        else if (scope == "music") app_scope |= 4;
        else if (scope == "photos") app_scope |= 8;
        else if (scope == "web") app_scope |= 16;
        else if (scope == "ai") app_scope |= 32;
    }
    return app_scope;
}

std::int16_t app_scope_from_json(const nlohmann::json& json)
{
    std::int16_t app_scope = 0;
    for (const auto& scope : json) {
        if (scope == "cloud") app_scope |= 1;
        else if (scope == "notes") app_scope |= 2;
        else if (scope == "music") app_scope |= 4;
        else if (scope == "photos") app_scope |= 8;
        else if (scope == "web") app_scope |= 16;
        else if (scope == "ai") app_scope |= 32;
    }
    return app_scope;
}

std::vector<std::string> app_scope_to_vector(std::int16_t app_scope)
{
    std::vector<std::string> scopes;
    scopes.reserve(6);

    if (app_scope & (1 << 0)) scopes.emplace_back("cloud");  // 1
    if (app_scope & (1 << 1)) scopes.emplace_back("notes");  // 2
    if (app_scope & (1 << 2)) scopes.emplace_back("music");  // 4
    if (app_scope & (1 << 3)) scopes.emplace_back("photos"); // 8
    if (app_scope & (1 << 4)) scopes.emplace_back("web");    // 16
    if (app_scope & (1 << 5)) scopes.emplace_back("ai");     // 32

    return scopes;
}

void sqlite_bind_item(sqlite3_stmt* stmt, const Item& item)
{
    auto id_bytes = item.id.to_bytes();
     sqlite3_bind_blob(stmt, 1, &id_bytes, sizeof(id_bytes), SQLITE_TRANSIENT);

    sqlite3_bind_text(stmt, 2, item_type::to_c_str(item.type), -1, SQLITE_TRANSIENT);

    sqlite3_bind_int64(stmt, 3, item.created_at);

    sqlite3_bind_int64(stmt, 4, item.updated_at);

    if (item.event_at.has_value()) {
        sqlite3_bind_int64(stmt, 5, item.event_at.value());
    } else {
        sqlite3_bind_null(stmt, 5);
    }

    if (item.deleted_at.has_value()) {
        sqlite3_bind_int64(stmt, 6, item.deleted_at.value());
    } else {
        sqlite3_bind_null(stmt, 6);
    }


    if (item.parent_id != special_folder::HOME && item.parent_id != special_folder::TRASH) {
        auto parent_id_bytes = item.parent_id.to_bytes();
        sqlite3_bind_blob(stmt, 7, &parent_id_bytes, sizeof(parent_id_bytes), SQLITE_TRANSIENT);
    } else {
        sqlite3_bind_null(stmt, 7);
    }


    if (!item.name.empty()) {
        sqlite3_bind_text(stmt, 8, item.name.c_str(), -1, SQLITE_TRANSIENT);
    } else {
        sqlite3_bind_null(stmt, 8);
    }

    sqlite3_bind_null(stmt, 9);

    if (item.comment.has_value()) {
        sqlite3_bind_text(stmt, 10, item.comment.value().c_str(), -1, SQLITE_TRANSIENT);
    } else {
        sqlite3_bind_null(stmt, 10);
    }

    sqlite3_bind_int(stmt, 11, item.encrypted ? 1 : 0);

    sqlite3_bind_int(stmt, 12, item.app_scope);
}

Item item_from_stmt(sqlite3_stmt* stmt)
{
    Item item;

    auto get_text_column = [](sqlite3_stmt* stmt, int col) -> std::string {
        if (sqlite3_column_type(stmt, col) == SQLITE_NULL) {
            return "";
        }
        const char* text = reinterpret_cast<const char*>(sqlite3_column_text(stmt, col));
        return text ? text : "";
    };

    auto get_int64_column = [](sqlite3_stmt* stmt, int col) -> std::optional<long long> {
        if (sqlite3_column_type(stmt, col) == SQLITE_NULL) {
            return std::nullopt;
        }
        return sqlite3_column_int64(stmt, col);
    };
    const void* id_bytes = sqlite3_column_blob(stmt, 0);
    item.id = ItemId::from_bytes(id_bytes);

    const char* type = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
    item.type = item_type::parse(type);
    item.created_at = sqlite3_column_int64(stmt, 2);
    item.updated_at = sqlite3_column_int64(stmt, 3);

    item.event_at    = get_int64_column(stmt, 4);
    item.deleted_at  = get_int64_column(stmt, 5);


    if (item.deleted_at.has_value())
    {
        if (sqlite3_column_type(stmt, 6) == SQLITE_BLOB)
        {
            const void* parent_id_bytes = sqlite3_column_blob(stmt, 6);
            item.parent_id = ItemId::from_bytes(parent_id_bytes);
        }
        else
        {
            item.parent_id = special_folder::TRASH;
        }
    }
    else
    {
        if (sqlite3_column_type(stmt, 6) == SQLITE_BLOB)
        {
            const void* parent_id_bytes = sqlite3_column_blob(stmt, 6);
            item.parent_id = ItemId::from_bytes(parent_id_bytes);
        }
        else
        {
            item.parent_id = special_folder::HOME;
        }
    }

    item.name        = get_text_column(stmt, 7);
    item.comment     = sqlite_utils::get_string(stmt, 8);

    item.encrypted = (sqlite3_column_int(stmt, 10) != 0);
    item.app_scope = static_cast<int16_t>(sqlite3_column_int(stmt, 11));
    item.cached = (sqlite3_column_int(stmt, 12) != 0);

    return item;
}

inline std::string parse_nullable_string(const nlohmann::json& json, const std::string& key) {
    if (json.contains(key) && !json.at(key).is_null()) {
        if (json.at(key).is_string()) {
            return json.at(key).get<std::string>();
        } else {
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
        item.id = ItemId::from_string(it->get<std::string>());
    }
    else
    {
        item.id = special_folder::HOME;
    }
    item.id = ItemId::from_string(json_utils::get_string(json, "id"));

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
        item.parent_id = ItemId::from_string(it->get<std::string>());
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

    if (json.contains("encrypted") && json["encrypted"].is_boolean()) {
        item.encrypted = json["encrypted"].get<bool>();
    } else {
        item.encrypted = false;
    }

    item.app_scope = 0;
    if (json.contains("app_scope") && json["app_scope"].is_array()) {
        for (const auto& scope : json["app_scope"]) {
            if (scope == "cloud") item.app_scope |= 1;
            else if (scope == "notes") item.app_scope |= 2;
            else if (scope == "music") item.app_scope |= 4;
            else if (scope == "photos") item.app_scope |= 8;
            else if (scope == "web") item.app_scope |= 16;
            else if (scope == "ai") item.app_scope |= 32;
        }
    }

    return item;
}
