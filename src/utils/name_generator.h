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
