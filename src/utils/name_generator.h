// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef SHARED_NAME_GENERATOR_H
#define SHARED_NAME_GENERATOR_H

#include <string>
#include <filesystem>

namespace fs = std::filesystem;

namespace name_generator {
    std::string generated_file_name(const std::string& type, const std::string& path);

    std::string generated_directory_name(const std::string& path);

    std::string generate_random_string();
};

#endif //SHARED_NAME_GENERATOR_H
