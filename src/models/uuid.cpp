// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include "uuid.h"

std::array<std::uint8_t, 16> UUID::to_bytes() const noexcept
{
    std::array<std::uint8_t, 16> bytes{};
    std::memcpy(bytes.data(), &high, 8);
    std::memcpy(bytes.data() + 8, &low, 8);
    return bytes;
}

UUID UUID::from_bytes(const void* blob_data) noexcept
{
    UUID id{};
    std::memcpy(&id.high, blob_data, 8);
    std::memcpy(&id.low, static_cast<const std::uint8_t*>(blob_data) + 8, 8);
    return id;
}

[[nodiscard]] constexpr std::int8_t hex_char_to_nibble(char c) noexcept {
    if (c >= '0' && c <= '9') return static_cast<int8_t>(c - '0');
    if (c >= 'a' && c <= 'f') return static_cast<int8_t>(c - 'a' + 10);
    if (c >= 'A' && c <= 'F') return static_cast<int8_t>(c - 'A' + 10);
    return -1;
}

UUID UUID::from_string(std::string_view str) noexcept
{
    if (str.size() != 36 && str.size() != 32)
    {
        return special_folder::UNKNOWN;
    }

    uint64_t high = 0;
    uint64_t low = 0;
    size_t hex_count = 0;

    for (char c : str)
    {
        if (c == '-')
        {
            continue;
        }

        const std::int8_t nibble = hex_char_to_nibble(c);
        if (nibble < 0)
        {
            return special_folder::UNKNOWN;
        }

        if (hex_count < 16)
        {
            high = (high << 4) | static_cast<uint64_t>(nibble);
        }
        else if (hex_count < 32)
        {
            low = (low << 4) | static_cast<uint64_t>(nibble);
        }
        else
        {
            return special_folder::UNKNOWN;
        }

        ++hex_count;
    }

    if (hex_count != 32)
    {
        return special_folder::UNKNOWN;
    }

    return UUID{high, low};
}
#if __cplusplus >= 202002L
std::strong_ordering UUID::operator<=>(const UUID& other) const
{
    if (auto cmp = this->high <=> other.high; cmp != 0)
    {
        return cmp;
    }
    return this->low <=> other.low;
}
#else
bool UUID::operator!=(const UUID& other) const noexcept
{
    return !(*this == other);
}
bool UUID::operator<(const UUID& other) const noexcept
{
    return std::tie(high, low) < std::tie(other.high, other.low);
}

bool UUID::operator<=(const UUID& other) const noexcept
{
    return other >= *this;
}

bool UUID::operator>(const UUID& other) const noexcept
{
    return other < *this;
}

bool UUID::operator>=(const UUID& other) const noexcept
{
    return !(*this < other);
}
#endif

bool UUID::operator==(const UUID& other) const
{
    return this->high == other.high && this->low == other.low;
}

[[nodiscard]] std::string UUID::to_string() const
{
    std::string result(36, '-');

    constexpr char hex_digits[] = "0123456789abcdef";


    size_t char_idx = 0;
    for (int i = 60; i >= 0; i -= 4)
    {
        if (char_idx == 8 || char_idx == 13)
        {
            ++char_idx;
        }
        uint8_t nibble = (high >> i) & 0x0F;
        result[char_idx++] = hex_digits[nibble];
    }


    ++char_idx;
    for (int i = 60; i >= 0; i -= 4)
    {
        if (char_idx == 23)
        {
            ++char_idx;
        }
        uint8_t nibble = (low >> i) & 0x0F;
        result[char_idx++] = hex_digits[nibble];
    }

    return result;
}
