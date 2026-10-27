
#include "network_provider_base.h"

#include <algorithm>
#include <array>
#include <filesystem>
#include <fstream>

#include "notifier_base.h"

void INetworkProviderBase::get(const std::string& url, const std::map<std::string, std::string>& headers,
    std::function<void(const amqhi::HttpResponse& response)> on_response,
    std::function<void(std::int16_t error_code, const std::string& data)> on_failure)
{
    get_impl(url, headers, [this, on_response = std::move(on_response)](amqhi::HttpResponse response)
    {
        m_callback_queue.emplace([on_response = on_response, response = std::move(response)]()
        {
            on_response(response);
        });
        process_callbacks();
    }, [this, on_failure = std::move(on_failure)](std::int16_t error_code, std::string data)
    {
        m_callback_queue.emplace([on_failure = on_failure, error_code, data = std::move(data)]()
        {
            on_failure(error_code, data);
        });
        process_callbacks();
    });
}

void INetworkProviderBase::post(const std::string& url, const std::map<std::string, std::string>& headers,
    std::function<void(const amqhi::HttpResponse& response)> on_response,
    std::function<void(std::int16_t error_code, const std::string& data)> on_failure)
{
    post_impl(url, headers, [this, on_response = std::move(on_response)](amqhi::HttpResponse response)
    {
        m_callback_queue.emplace([on_response = on_response, response = std::move(response)]()
        {
            on_response(response);
        });
        process_callbacks();
    }, [this, on_failure = std::move(on_failure)](std::int16_t error_code, std::string data)
    {
        m_callback_queue.emplace([on_failure = on_failure, error_code, data = std::move(data)]()
        {
            on_failure(error_code, data);
        });
        process_callbacks();
    });
}

void INetworkProviderBase::post(const std::string& url, const std::map<std::string, std::string>& headers,
    const nlohmann::json& body, std::function<void(const amqhi::HttpResponse& response)> on_response,
    std::function<void(std::int16_t error_code, const std::string& data)> on_failure)
{
    post_impl(url, headers, body, [this, on_response = std::move(on_response)](amqhi::HttpResponse response)
    {
        m_callback_queue.emplace([on_response = on_response, response = std::move(response)]()
        {
            on_response(response);
        });
        process_callbacks();
    }, [this, on_failure = std::move(on_failure)](std::int16_t error_code, std::string data)
    {
        m_callback_queue.emplace([on_failure = on_failure, error_code, data = std::move(data)]()
        {
            on_failure(error_code, data);
        });
        process_callbacks();
    });
}

void INetworkProviderBase::put_file(const std::string& url, const std::map<std::string, std::string>& headers,
    const std::string& file_path, const nlohmann::json& task_info,
    std::function<void(const amqhi::HttpResponse& response)> on_response,
    std::function<void(std::int16_t error_code, const std::string& data)> on_failure)
{
    put_file_impl(url, headers, file_path, task_info, [this, on_response = std::move(on_response)](amqhi::HttpResponse response)
    {
        m_callback_queue.emplace([on_response = on_response, response = std::move(response)]()
        {
            on_response(response);
        });
        process_callbacks();
    }, [this, on_failure = std::move(on_failure)](std::int16_t error_code, std::string data)
    {
        m_callback_queue.emplace([on_failure = on_failure, error_code, data = std::move(data)]()
        {
            on_failure(error_code, data);
        });
        process_callbacks();
    });
}

void INetworkProviderBase::put_file(const nlohmann::json& parts, const std::string& mime_type, const std::string& file_path, const nlohmann::json& task_info, std::function<void(const amqhi::HttpResponse& response)> on_response, std::function<void(std::int16_t error_code, const std::string& data)> on_failure)
{
    if (!parts.is_array())
    {
        return;
    }
    auto multipart_file_task = new PutMultipartFileTask();
    multipart_file_task->mime_type = mime_type;
    multipart_file_task->on_response = std::move(on_response);
    multipart_file_task->on_failure = std::move(on_failure);
    multipart_file_task->task_info = task_info;
    for (const auto & it : parts)
    {
        MultipartPart part;
        part.url = it["url"];
        part.part_number = it["part_number"];
        part.size = it["size"];
        part.start_offset = it["start_offset"];
        part.file_path = file_path + "_" + std::to_string(part.part_number);

        std::error_code ec;
        const auto source_size = std::filesystem::file_size(file_path, ec);
        const bool range_out_of_file = part.start_offset < 0 || part.size < 0
            || static_cast<std::uint64_t>(part.start_offset) > source_size
            || static_cast<std::uint64_t>(part.size) > source_size - static_cast<std::uint64_t>(part.start_offset);
        if (ec || range_out_of_file)
        {
            on_failure(FILE_CREATE_FAILURE, "failed to create multipart part file: " + part.file_path);
            delete multipart_file_task;
            return;
        }

        std::ifstream input(file_path, std::ios::binary);
        input.seekg(part.start_offset);
        std::ofstream output(part.file_path, std::ios::binary | std::ios::trunc);
        if (!input || !output)
        {
            on_failure(FILE_CREATE_FAILURE, "failed to create multipart part file: " + part.file_path);
            delete multipart_file_task;
            return;
        }

        std::array<char, 8192> buffer{};
        std::int64_t remaining = part.size;
        while (remaining > 0)
        {
            const auto chunk = static_cast<std::streamsize>(std::min<std::int64_t>(remaining, buffer.size()));
            input.read(buffer.data(), chunk);
            const auto read_count = input.gcount();
            if (read_count <= 0)
            {
                on_failure(FILE_READ_FAILURE, "failed to read multipart part file: " + part.file_path);
                delete multipart_file_task;
                return;
            }
            output.write(buffer.data(), read_count);
            if (!output)
            {
                on_failure(FILE_WRITE_FAILURE, "failed to write multipart part file: " + part.file_path);
                delete multipart_file_task;
                return;
            }
            remaining -= read_count;
        }

        multipart_file_task->parts.push_back(std::move(part));
    }
    put_next_file(multipart_file_task, 0);
}

void INetworkProviderBase::download_file(const std::string& url, const std::map<std::string, std::string>& headers,
    const std::string& file_path, const nlohmann::json& task_info,
    std::function<void(const amqhi::HttpResponse& response)> on_response,
    std::function<void(std::int16_t error_code, const std::string& data)> on_failure)
{
    download_file_impl(url, headers, file_path, task_info, [this, on_response = std::move(on_response)](amqhi::HttpResponse response)
    {
        m_callback_queue.emplace([on_response = on_response, response = std::move(response)]()
        {
            on_response(response);
        });
        process_callbacks();
    }, [this, on_failure = std::move(on_failure)](std::int16_t error_code, std::string data)
    {
        m_callback_queue.emplace([on_failure = on_failure, error_code, data = std::move(data)]()
        {
            on_failure(error_code, data);
        });
        process_callbacks();
    });
}

void INetworkProviderBase::patch(const std::string& url, const std::map<std::string, std::string>& headers,
    const nlohmann::json& body, std::function<void(const amqhi::HttpResponse& response)> on_response,
    std::function<void(std::int16_t error_code, const std::string& data)> on_failure)
{
    patch_impl(url, headers, body, [this, on_response = std::move(on_response)](amqhi::HttpResponse response)
    {
        m_callback_queue.emplace([on_response = on_response, response = std::move(response)]()
        {
            on_response(response);
        });
        process_callbacks();
    }, [this, on_failure = std::move(on_failure)](std::int16_t error_code, std::string data)
    {
        m_callback_queue.emplace([on_failure = on_failure, error_code, data = std::move(data)]()
        {
            on_failure(error_code, data);
        });
        process_callbacks();
    });
}

void INetworkProviderBase::put(const std::string& url, const std::map<std::string, std::string>& headers,
    const nlohmann::json& body, std::function<void(const amqhi::HttpResponse& response)> on_response,
    std::function<void(std::int16_t error_code, const std::string& data)> on_failure)
{
    put_impl(url, headers, body, [this, on_response = std::move(on_response)](amqhi::HttpResponse response)
    {
        m_callback_queue.emplace([on_response = on_response, response = std::move(response)]()
        {
            on_response(response);
        });
        process_callbacks();
    }, [this, on_failure = std::move(on_failure)](std::int16_t error_code, std::string data)
    {
        m_callback_queue.emplace([on_failure = on_failure, error_code, data = std::move(data)]()
        {
            on_failure(error_code, data);
        });
        process_callbacks();
    });
}

void INetworkProviderBase::destroy(const std::string& url, const std::map<std::string, std::string>& headers,
                                   std::function<void(const amqhi::HttpResponse& response)> on_response,
                                   std::function<void(std::int16_t error_code, const std::string& data)> on_failure)
{
    destroy_impl(url, headers, [this, on_response = std::move(on_response)](amqhi::HttpResponse response)
    {
        m_callback_queue.emplace([on_response = on_response, response = std::move(response)]()
        {
            on_response(response);
        });
        process_callbacks();
    }, [this, on_failure = std::move(on_failure)](std::int16_t error_code, std::string data)
    {
        m_callback_queue.emplace([on_failure = on_failure, error_code, data = std::move(data)]()
        {
            on_failure(error_code, data);
        });
        process_callbacks();
    });
}

void INetworkProviderBase::put_next_file(PutMultipartFileTask* task, std::size_t index)
{
    auto& part = task->parts[index];
    std::map<std::string, std::string> headers;
    headers["Content-Type"] = task->mime_type;

    put_file(part.url, headers, part.file_path, task->task_info, [this, task, index](const amqhi::HttpResponse& response)
    {
        if (response.status_code == 200)
        {
            nlohmann::json composite_result;
            composite_result["part_number"] =  task->parts[index].part_number;
            composite_result["etag"] = response.headers.at("ETag");
            task->composite_results.push_back(composite_result);
        }
        else
        {
            m_callback_queue.emplace([task, response]()
             {
                task->on_response(response);
                delete task;
             });
            process_callbacks();
            return;
        }
        if (index == task->parts.size() - 1)
        {
            amqhi::HttpResponse composite_results_response;
            composite_results_response.status_code = 200;
            composite_results_response.body = task->composite_results.dump();
            m_callback_queue.emplace([composite_results_response = std::move(composite_results_response), task]()
 {
                task->on_response(composite_results_response);
                delete task;
 });
            process_callbacks();
            return;
        }
        put_next_file(task, index + 1);
    },
    [this, task](std::int16_t error_code, const std::string& data)
             {
        m_callback_queue.emplace([error_code, data, task]()
        {
            task->on_failure(error_code, data);
            delete task;
        });
        process_callbacks();
             });
}

void INetworkProviderBase::process_callbacks()
{
    if (m_is_processing_callbacks)
    {
        return;
    }

    m_is_processing_callbacks = true;

    while (!m_callback_queue.empty())
    {
        auto callback = std::move(m_callback_queue.front());
        m_callback_queue.pop();

        callback();
    }

    m_is_processing_callbacks = false;
    if (!m_callback_queue.empty())
    {
        process_callbacks();
    }
}


