// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

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
