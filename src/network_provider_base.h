// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef SHARED_NETWORK_PROVIDER_H
#define SHARED_NETWORK_PROVIDER_H

#include <functional>
#include <string>
#include <map>
#include "json.hpp"

namespace transfer_type
{
    constexpr std::int8_t FILE = 0;
    constexpr std::int8_t THUMBNAIL = 1;
}

class INetworkProviderBase
{
public:
    virtual ~INetworkProviderBase() = default;
    virtual void get(
        const std::string& url,
        const std::map<std::string, std::string>& headers,
        const std::function<void(int status_code, const std::string& response)>& on_response,
        const std::function<void(std::int16_t error_code, const std::string& data)>& on_failure
    ) = 0;
    virtual void post(
     const std::string& url,
     const std::map<std::string, std::string>& headers,
     const std::function<void(int status_code, const std::string& response)>& on_response,
     const std::function<void(std::int16_t error_code, const std::string& data)>& on_failure
 ) = 0;
    virtual void post_json(
        const std::string& url,
        const std::map<std::string, std::string>& headers,
        const nlohmann::json& body,
        const std::function<void(int status_code, const std::string& response)>& on_response,
        const std::function<void(std::int16_t error_code, const std::string& data)>& on_failure
    ) = 0;
    virtual void put_file(
          const std::string& url,
          const std::map<std::string, std::string>& headers,
          const std::string& file_path,
          const nlohmann::json& task_info,
          const std::function<void(int status_code, const std::string& response)>& on_response,
          const std::function<void(std::int16_t error_code, const std::string& data)>& on_failure
      ) = 0;
    virtual void put_file(
      const nlohmann::json& parts,
      const std::string& mime_type,
      const std::string& file_path,
      const nlohmann::json& task_info,
      const std::function<void(int status_code, const std::string& response)>& on_response,
      const std::function<void(std::int16_t error_code, const std::string& data)>& on_failure
  ) = 0;
    virtual void download_file(
            const std::string& url,
            const std::map<std::string, std::string>& headers,
            const std::string& file_path,
            const nlohmann::json& task_info,
            const std::function<void(int status_code, const std::string& response)>& on_response,
            const std::function<void(std::int16_t error_code, const std::string& data)>& on_failure
    ) = 0;
    virtual void patch_json(
        const std::string& url,
        const std::map<std::string, std::string>& headers,
        const nlohmann::json& body,
        const std::function<void(int status_code, const std::string& response)>& on_response,
        const std::function<void(std::int16_t error_code, const std::string& data)>& on_failure
    ) = 0;
    virtual void destroy(
        const std::string& url,
        const std::map<std::string, std::string>& headers,
        const std::function<void(int status_code, const std::string& response)>& on_response,
        const std::function<void(std::int16_t error_code, const std::string& data)>& on_failure
    ) = 0;
};

#endif //SHARED_NETWORK_PROVIDER_H
