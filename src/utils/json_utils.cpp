// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include "json_utils.h"

std::string json_utils::get_string(const nlohmann::json& j, const std::string& key)
{
    return get_string(j, key, "");
}

std::string json_utils::get_string(const nlohmann::json& j, const std::string& key, const std::string& fallback)
{
    if (auto it = j.find(key); it != j.end() && it->is_string())
    {
        return it->get<std::string>();
    }
    return fallback;
}

std::int64_t json_utils::get_int64_t(const nlohmann::json& j, const std::string& key, std::int64_t fallback)
{
    if (auto it = j.find(key); it != j.end() && it->is_number_integer())
    {
        return it->get<int64_t>();
    }
    return fallback;
}

std::int64_t json_utils::get_int64_t(const nlohmann::json& j, const std::string& key)
{
    return get_int64_t(j, key, -1);
}

std::int32_t json_utils::get_int32_t(const nlohmann::json& j, const std::string& key, std::int32_t fallback)
{
    if (auto it = j.find(key); it != j.end() && it->is_number_integer())
    {
        return it->get<int32_t>();
    }
    return fallback;
}

std::int32_t json_utils::get_int32_t(const nlohmann::json& j, const std::string& key)
{
    return get_int32_t(j, key, -1);
}

std::int16_t json_utils::get_int16_t(const nlohmann::json& j, const std::string& key, std::int16_t fallback)
{
    if (auto it = j.find(key); it != j.end() && it->is_number_integer())
    {
        return it->get<int16_t>();
    }
    return fallback;
}

std::int16_t json_utils::get_int16_t(const nlohmann::json& j, const std::string& key)
{
    return get_int16_t(j, key, -1);
}

std::int8_t json_utils::get_int8_t(const nlohmann::json& j, const std::string& key, std::int8_t fallback)
{
    if (auto it = j.find(key); it != j.end() && it->is_number_integer())
    {
        return it->get<int8_t>();
    }
    return fallback;
}

std::int8_t json_utils::get_int8_t(const nlohmann::json& j, const std::string& key)
{
    return get_int8_t(j, key, -1);
}

bool json_utils::get_bool(const nlohmann::json& j, const std::string& key, bool fallback)
{
    if (auto it = j.find(key); it != j.end() && it->is_boolean())
    {
        return it->get<bool>();
    }
    return fallback;
}

bool json_utils::get_bool(const nlohmann::json& j, const std::string& key)
{
    return get_bool(j, key, false);
}
