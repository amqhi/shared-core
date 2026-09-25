
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef SHARED_CORE_UUID_H
#define SHARED_CORE_UUID_H

#include <array>
#include <string_view>
#include <string>

struct UUID
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

    static UUID from_bytes(const void* blob_data) noexcept;

    [[nodiscard]] static UUID from_string(std::string_view str) noexcept;

#if __cplusplus >= 202002L
    std::strong_ordering operator<=>(const UUID& other) const;
#else
    bool operator!=(const UUID& other) const noexcept;
    bool operator<(const UUID& other) const noexcept;
    bool operator<=(const UUID& other) const noexcept;
    bool operator>(const UUID& other) const noexcept;
    bool operator>=(const UUID& other) const noexcept;

#endif
    bool operator==(const UUID& other) const;

    [[nodiscard]] std::string to_string() const;
};

static_assert(sizeof(UUID) == 16);

// Custom std::hash specialization for UUID (for std::unordered_map)
template <>
struct std::hash<UUID> {
    std::size_t operator()(const UUID& id) const noexcept {
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
    constexpr UUID HOME    = {0x1111111111111111ULL, 0x1111111111111111ULL};
    constexpr UUID TRASH   = {0x1111111111111111ULL, 0x1111111111111112ULL};
    constexpr UUID UNKNOWN = {0x0000000000000000ULL, 0x0000000000000000ULL};
}

#endif //SHARED_CORE_UUID_H
