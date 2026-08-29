#ifndef SHARED_JSON_UTILS_H
#define SHARED_JSON_UTILS_H

#include "json.hpp"

namespace json_utils
{
    std::string get_string(const nlohmann::json& j, const std::string& key);
    std::string get_string(const nlohmann::json& j, const std::string& key, const std::string& fallback);

    template<typename NumberType>
    NumberType get_int(const nlohmann::json& j, const std::string& key, NumberType fallback)
    {
        if (auto it = j.find(key); it != j.end() && it->is_number_integer())
        {
            return it->get<NumberType>();
        }
        return fallback;
    }

    template<typename NumberType>
    NumberType get_int(const nlohmann::json& j, const std::string& key)
    {
        return get_int(j, key, -1);
    }

    std::int64_t get_int64_t(const nlohmann::json& j, const std::string& key, std::int64_t fallback);
    std::int64_t get_int64_t(const nlohmann::json& j, const std::string& key);

    std::int32_t get_int32_t(const nlohmann::json& j, const std::string& key, std::int32_t fallback);
    std::int32_t get_int32_t(const nlohmann::json& j, const std::string& key);

    std::int16_t get_int16_t(const nlohmann::json& j, const std::string& key, std::int16_t fallback);
    std::int16_t get_int16_t(const nlohmann::json& j, const std::string& key);

    std::int8_t get_int8_t(const nlohmann::json& j, const std::string& key, std::int8_t fallback);
    std::int8_t get_int8_t(const nlohmann::json& j, const std::string& key);
}

#endif //SHARED_JSON_UTILS_H
