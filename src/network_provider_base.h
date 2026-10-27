// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef SHARED_NETWORK_PROVIDER_H
#define SHARED_NETWORK_PROVIDER_H

#include <functional>
#include <string>
#include <map>
#include "json.hpp"
#include <queue>

namespace transfer_type
{
    constexpr std::int8_t FILE = 0;
    constexpr std::int8_t THUMBNAIL = 1;
    constexpr std::int8_t MULTIPART_FILE = 2;
}

struct MultipartPart
{
    std::string url;
    std::string file_path;
    std::int32_t part_number;
    std::int64_t start_offset;
    std::int64_t size;
};

namespace amqhi
{
    struct HttpResponse
    {
        std::unordered_map<std::string, std::string> headers;
        std::string body;
        int status_code;
    };
}

struct PutMultipartFileTask
{
    std::vector<MultipartPart> parts;
    std::string mime_type;
    /// Contains each part's ETag and part number
    nlohmann::json composite_results;
    nlohmann::json task_info;
    std::function<void(const amqhi::HttpResponse& response)> on_response;
    std::function<void(std::int16_t error_code, const std::string& data)> on_failure;
};

class INetworkProviderBase
{
public:
    virtual ~INetworkProviderBase() = default;
    void get(
        const std::string& url,
        const std::map<std::string, std::string>& headers,
        std::function<void(const amqhi::HttpResponse& response)> on_response,
        std::function<void(std::int16_t error_code, const std::string& data)> on_failure
    );
    void post(
        const std::string& url,
        const std::map<std::string, std::string>& headers,
        std::function<void(const amqhi::HttpResponse& response)> on_response,
        std::function<void(std::int16_t error_code, const std::string& data)> on_failure
    );
    void post(
        const std::string& url,
        const std::map<std::string, std::string>& headers,
        const nlohmann::json& body,
        std::function<void(const amqhi::HttpResponse& response)> on_response,
        std::function<void(std::int16_t error_code, const std::string& data)> on_failure
    );
    void put_file(
        const std::string& url,
        const std::map<std::string, std::string>& headers,
        const std::string& file_path,
        const nlohmann::json& task_info,
        std::function<void(const amqhi::HttpResponse& response)> on_response,
        std::function<void(std::int16_t error_code, const std::string& data)> on_failure
    );
    void put_file(
        const nlohmann::json& parts,
        const std::string& mime_type,
        const std::string& file_path,
        const nlohmann::json& task_info,
        std::function<void(const amqhi::HttpResponse& response)> on_response,
        std::function<void(std::int16_t error_code, const std::string& data)> on_failure
    );
    void download_file(
        const std::string& url,
        const std::map<std::string, std::string>& headers,
        const std::string& file_path,
        const nlohmann::json& task_info,
        std::function<void(const amqhi::HttpResponse& response)> on_response,
        std::function<void(std::int16_t error_code, const std::string& data)> on_failure
    );
    void patch(
        const std::string& url,
        const std::map<std::string, std::string>& headers,
        const nlohmann::json& body,
        std::function<void(const amqhi::HttpResponse& response)> on_response,
        std::function<void(std::int16_t error_code, const std::string& data)> on_failure
    );
    void put(
        const std::string& url,
        const std::map<std::string, std::string>& headers,
        const nlohmann::json& body,
        std::function<void(const amqhi::HttpResponse& response)> on_response,
        std::function<void(std::int16_t error_code, const std::string& data)> on_failure
    );
    void destroy(
        const std::string& url,
        const std::map<std::string, std::string>& headers,
        std::function<void(const amqhi::HttpResponse& response)> on_response,
        std::function<void(std::int16_t error_code, const std::string& data)> on_failure
    );

protected:
    virtual void get_impl(
        const std::string& url,
        const std::map<std::string, std::string>& headers,
        const std::function<void(const amqhi::HttpResponse& response)>& on_response,
        const std::function<void(std::int16_t error_code, const std::string& data)>& on_failure
    ) = 0;
    virtual void post_impl(
        const std::string& url,
        const std::map<std::string, std::string>& headers,
        const std::function<void(const amqhi::HttpResponse& response)>& on_response,
        const std::function<void(std::int16_t error_code, const std::string& data)>& on_failure
    ) = 0;
    virtual void post_impl(
        const std::string& url,
        const std::map<std::string, std::string>& headers,
        const nlohmann::json& body,
        const std::function<void(const amqhi::HttpResponse& response)>& on_response,
        const std::function<void(std::int16_t error_code, const std::string& data)>& on_failure
    ) = 0;
    virtual void put_file_impl(
        const std::string& url,
        const std::map<std::string, std::string>& headers,
        const std::string& file_path,
        const nlohmann::json& task_info,
        const std::function<void(const amqhi::HttpResponse& response)>& on_response,
        const std::function<void(std::int16_t error_code, const std::string& data)>& on_failure
    ) = 0;
    virtual void download_file_impl(
        const std::string& url,
        const std::map<std::string, std::string>& headers,
        const std::string& file_path,
        const nlohmann::json& task_info,
        const std::function<void(const amqhi::HttpResponse& response)>& on_response,
        const std::function<void(std::int16_t error_code, const std::string& data)>& on_failure
    ) = 0;
    virtual void patch_impl(
        const std::string& url,
        const std::map<std::string, std::string>& headers,
        const nlohmann::json& body,
        const std::function<void(const amqhi::HttpResponse& response)>& on_response,
        const std::function<void(std::int16_t error_code, const std::string& data)>& on_failure
    ) = 0;
    virtual void put_impl(
        const std::string& url,
        const std::map<std::string, std::string>& headers,
        const nlohmann::json& body,
        const std::function<void(const amqhi::HttpResponse& response)>& on_response,
        const std::function<void(std::int16_t error_code, const std::string& data)>& on_failure
    ) = 0;
    virtual void destroy_impl(
        const std::string& url,
        const std::map<std::string, std::string>& headers,
        const std::function<void(const amqhi::HttpResponse& response)>& on_response,
        const std::function<void(std::int16_t error_code, const std::string& data)>& on_failure
    ) = 0;

    void put_next_file(PutMultipartFileTask* task, size_t index);
    void process_callbacks();
    std::queue<std::function<void ()>> m_callback_queue;
    bool m_is_processing_callbacks = false;
};

#endif //SHARED_NETWORK_PROVIDER_H
