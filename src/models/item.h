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

[[nodiscard]] constexpr std::int8_t hex_char_to_nibble(char c) noexcept {
    if (c >= '0' && c <= '9') return static_cast<int8_t>(c - '0');
    if (c >= 'a' && c <= 'f') return static_cast<int8_t>(c - 'a' + 10);
    if (c >= 'A' && c <= 'F') return static_cast<int8_t>(c - 'A' + 10);
    return -1;
}

struct ItemId
{
    std::uint64_t high;
    std::uint64_t low;

    // 128-bit UUID representation:
    // A 32-hex-digit UUID string is parsed into two 64-bit unsigned integers without hyphens.
    // Each hex character represents a 4-bit nibble and is shifted left into position:
    //
    // UUID: "123e4567-e89b-12d3-a456-426614174000"
    //
    // high (64-bit): [ 1 ][ 2 ][ 3 ][ e ] ... [ d ][ 3 ]  (First 16 hex digits, 0x123e4567e89b12d3)
    // low  (64-bit): [ a ][ 4 ][ 5 ][ 6 ] ... [ 0 ][ 0 ]  (Last 16 hex digits,  0xa456426614174000)
    //                |<- 4 bits per char ->|

    [[nodiscard]] std::array<std::uint8_t, 16> to_bytes() const noexcept;

    static ItemId from_bytes(const void* blob_data) noexcept;

    [[nodiscard]] static ItemId from_string(std::string_view str) noexcept;

#if __cplusplus >= 202002L
    std::strong_ordering operator<=>(const ItemId& other) const;
#else
    bool operator!=(const ItemId& other) const noexcept;
    bool operator<(const ItemId& other) const noexcept;
    bool operator<=(const ItemId& other) const noexcept;
    bool operator>(const ItemId& other) const noexcept;
    bool operator>=(const ItemId& other) const noexcept;

#endif
    bool operator==(const ItemId& other) const;
    
    [[nodiscard]] std::string to_string() const;
};

static_assert(sizeof(ItemId) == 16);

// Custom std::hash specialization for ItemId (for std::unordered_map)
template <>
struct std::hash<ItemId> {
    std::size_t operator()(const ItemId& id) const noexcept {
        std::size_t h1 = std::hash<std::uint64_t>{}(id.high);
        std::size_t h2 = std::hash<std::uint64_t>{}(id.low);
        // 0x9e3779b97f4a7c15ULL represents the golden ratio to prevent hash collisions
        return h1 ^ (h2 + 0x9e3779b97f4a7c15ULL + (h1 << 6) + (h1 >> 2));
    }
};

// It's used only as virtual identifiers/parameters.
// It's not stored directly in the Item's parent_id field.
namespace special_folder
{
    constexpr ItemId HOME    = {0x1111111111111111ULL, 0x1111111111111111ULL};
    constexpr ItemId TRASH   = {0x1111111111111111ULL, 0x1111111111111112ULL};
    constexpr ItemId UNKNOWN = {0x0000000000000000ULL, 0x0000000000000000ULL};
}


struct Item {
    std::string name;
    std::optional<std::string> comment = std::nullopt;
    ItemId id;
    ItemId parent_id;
    std::int64_t created_at = 0;
    std::int64_t updated_at = 0;
    std::optional<std::int64_t> event_at = std::nullopt;
    std::optional<std::int64_t> deleted_at = std::nullopt;

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
};


void item_delete_on_local(const std::string& app_support_path, std::uint8_t user_id, sqlite3* db, const ItemId& item_id);

std::filesystem::path item_local_directory_path(const std::string& app_support_path, std::uint8_t user_id, const ItemId& id);
std::filesystem::path item_local_file_path(const std::string& app_support_path, std::uint8_t user_id, const ItemId& id);
std::filesystem::path item_thumbnail_path(const std::string& app_support_path, std::uint8_t user_id, const ItemId& id);

std::int16_t app_scope_from_vector(const std::vector<std::string>& app_scope);
std::int16_t app_scope_from_json(const nlohmann::json& app_scope);
std::vector<std::string> app_scope_to_vector(std::int16_t app_scope);

Item item_from_json(const nlohmann::json& json);

void sqlite_bind_item(sqlite3_stmt* stmt, const Item& item);
Item item_from_stmt(sqlite3_stmt* stmt);

#endif //SHARED_ITEM_H
