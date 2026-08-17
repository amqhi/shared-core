#ifndef SHARED_NETWORK_PROVIDER_H
#define SHARED_NETWORK_PROVIDER_H

#include <functional>
#include <string>
#include <map>
#include "json.hpp"

namespace transfer_type
{
    constexpr std::int8_t FILE = 0;
}

class INetworkProviderBase
{
public:
    virtual ~INetworkProviderBase() = default;
    virtual void get(
        const std::string& url,
        const std::map<std::string, std::string>& headers,
        std::function<void(int status_code, const std::string& response)> on_response,
        std::function<void(std::int16_t error_code, const std::string& data)> on_failure
    ) = 0;
    virtual void post(
     const std::string& url,
     const std::map<std::string, std::string>& headers,
     std::function<void(int status_code, const std::string& response)> on_response,
     std::function<void(std::int16_t error_code, const std::string& data)> on_failure
 ) = 0;
    virtual void post_json(
        const std::string& url,
        const std::map<std::string, std::string>& headers,
        const nlohmann::json& body,
        std::function<void(int status_code, const std::string& response)> on_response,
        std::function<void(std::int16_t error_code, const std::string& data)> on_failure
    ) = 0;
    virtual void put_file(
          const std::string& url,
          const std::map<std::string, std::string>& headers,
          const std::string& file_path,
          std::function<void(int64_t bytes_written, int64_t total_bytes)> on_progress,
          std::function<void(int status_code, const std::string& response)> on_response,
          std::function<void(std::int16_t error_code, const std::string& data)> on_failure
      ) = 0;
    virtual void put_file(
      const nlohmann::json& parts,
      const std::string& mime_type,
      const std::string& file_path,
      std::function<void(int64_t bytes_written, int64_t total_bytes)> on_progress,
      std::function<void(int status_code, const std::string& response)> on_response,
      std::function<void(std::int16_t error_code, const std::string& data)> on_failure
  ) = 0;
    virtual void download_file(
            const std::string& url,
            const std::map<std::string, std::string>& headers,
            const std::string& file_path,
            std::function<void(int64_t bytes_received, int64_t total_bytes)> on_progress,
            std::function<void(int status_code, const std::string& response)> on_response,
            std::function<void(std::int16_t error_code, const std::string& data)> on_failure
    ) = 0;
    virtual void patch_json(
        const std::string& url,
        const std::map<std::string, std::string>& headers,
        const nlohmann::json& body,
        std::function<void(int status_code, const std::string& response)> on_response,
        std::function<void(std::int16_t error_code, const std::string& data)> on_failure
    ) = 0;
    virtual void destroy(
        const std::string& url,
        const std::map<std::string, std::string>& headers,
        std::function<void(int status_code, const std::string& response)> on_response,
        std::function<void(std::int16_t error_code, const std::string& data)> on_failure
    ) = 0;
};

#endif //SHARED_NETWORK_PROVIDER_H
