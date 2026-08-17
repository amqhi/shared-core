#ifndef SHARED_ITEM_H
#define SHARED_ITEM_H

#include <string>
#include <optional>
#include "json.hpp"
#include "sqlite3.h"
#include <filesystem>
#include "item_attributes.h"

// It's used only as virtual identifiers/parameters.
// It's not stored directly in the Item's parent_id field.
namespace special_folder
{
   constexpr const char* const TRASH = "trash";
   constexpr const char* const HOME = "home";
}

struct ItemAttributes;
struct User;

namespace item_type
{
    constexpr std::int8_t FILE = 0;
    constexpr std::int8_t FOLDER = 1;
    constexpr std::int8_t NOTE = 2;
    constexpr std::int8_t SONG = 3;
    constexpr std::int8_t ARTIST = 4;
    constexpr std::int8_t ALBUM = 5;
    constexpr std::int8_t ALIAS = 6;
    constexpr std::int8_t PHOTO = 7;
    constexpr std::int8_t UNKNOWN = 8;

    std::string to_string(std::int8_t type);
    const char* to_c_str(std::int8_t type);
    std::int8_t parse(std::string_view type);
}

struct Item {
    std::string id;
    std::string parent_id;
    std::string name;
    std::optional<std::string> comment = std::nullopt;
    std::optional<int64_t> event_at = std::nullopt;
    std::optional<int64_t> deleted_at = std::nullopt;
    std::int64_t created_at = 0;
    std::int64_t updated_at = 0;

    /**
       * Bitmask representing which applications can view/access this item.
       * Default value is 63 (0b111111: all scopes enabled).
       */
    std::int16_t app_scope = 63;
    std::int8_t type;
    bool encrypted = false;
    bool cached = false;

    [[nodiscard]] bool is_deleted() const {
        return deleted_at > 0;
    }

    void save(sqlite3* db) const;

    void update(const ItemAttributes& item_attributes);
};

void item_delete_on_local(const std::string& app_support_path, const std::string& user_id, sqlite3* db, const std::string& item_id);

std::filesystem::path item_local_directory_path(const std::string& app_support_path, const std::string& user_id, const std::string& id);
std::filesystem::path item_local_file_path(const std::string& app_support_path, const std::string& user_id, const std::string& id);
std::filesystem::path item_thumbnail_path(const std::string& app_support_path, const std::string& user_id, const std::string& id);

std::int16_t app_scope_from_vector(const std::vector<std::string>& app_scope);
std::int16_t app_scope_from_json(const nlohmann::json& app_scope);
std::vector<std::string> app_scope_to_vector(std::int16_t app_scope);

Item item_from_json(const nlohmann::json& json);

void sqlite_bind_item(sqlite3_stmt* stmt, const Item& item);
Item item_from_stmt(sqlite3_stmt* stmt);



struct ItemSummary
{
    std::string id;
    std::string type;
    std::string name;
    std::string thumbnail_path;
};

#endif //SHARED_ITEM_H
