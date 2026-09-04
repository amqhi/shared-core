// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include "name_generator.h"
#include <random>

std::string name_generator::generated_file_name(const std::string& type, const std::string& path)
{
    std::string result = generate_random_string();
    result += type;

    const fs::path full_path = fs::path(path) / result;

    if (fs::exists(full_path)) {
        return generated_file_name(type, path);
    }
    return result;
}
std::string name_generator::generated_directory_name(const std::string& path)
{
    std::string result = generate_random_string();

    fs::path full_Path = fs::path(path) / result;

    if (fs::exists(full_Path)) {
        return generated_directory_name(path);
    }
    return result;
}
std::string name_generator::generate_random_string() {
    const std::string chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

    static std::random_device rd;
    static std::mt19937 gen(rd());

    std::uniform_int_distribution<> lenDist(10, 14);
    int length = lenDist(gen);

    std::uniform_int_distribution<> charDist(0, chars.length() - 1);

    std::string res;
    res.reserve(length);
    for (int i = 0; i < length; ++i) {
        res += chars[charDist(gen)];
    }
    return res;
}
