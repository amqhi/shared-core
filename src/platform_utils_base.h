#ifndef SHARED_PLATFORM_UTILS_H
#define SHARED_PLATFORM_UTILS_H
#include <string>

class IPlatformUtilsBase
{
public:
    virtual ~IPlatformUtilsBase() = default;
    [[nodiscard]] virtual std::string get_file_sha256_checksum(const std::string& file_path) const = 0;
    [[nodiscard]] virtual std::string get_device_name() const = 0;
};

#endif //SHARED_PLATFORM_UTILS_H
