
#include "network_provider_base.h"

#include <algorithm>
#include <array>
#include <filesystem>
#include <fstream>

#include "notifier_base.h"

void INetworkProviderBase::put_file(const nlohmann::json& parts, const std::string& mime_type, const std::string& file_path, const nlohmann::json& task_info, const std::function<void(const amqhi::HttpResponse& response)>& on_response, const std::function<void(std::int16_t error_code, const std::string& data)>& on_failure)
{
    if (m_put_multipart_file_task)
    {
        return;
    }
    m_put_multipart_file_task = std::make_unique<PutMultipartFileTask>();

    if (!parts.is_array())
    {
        m_put_multipart_file_task.reset();
        return;
    }

    for (const auto & it : parts)
    {
        MultipartPart part;
        part.part_number = it["part_number"];
        part.size = it["size"];
        part.start_offset = it["start_offset"];
        part.url = it["url"];
        part.file_path = file_path + "_" + std::to_string(part.part_number);

        std::error_code ec;
        const auto source_size = std::filesystem::file_size(file_path, ec);
        const bool range_out_of_file = part.start_offset < 0 || part.size < 0
            || static_cast<std::uint64_t>(part.start_offset) > source_size
            || static_cast<std::uint64_t>(part.size) > source_size - static_cast<std::uint64_t>(part.start_offset);
        if (ec || range_out_of_file)
        {
            m_put_multipart_file_task.reset();
            on_failure(FILE_CREATE_FAILURE, "failed to create multipart part file: " + part.file_path);
            return;
        }

        std::ifstream input(file_path, std::ios::binary);
        input.seekg(part.start_offset);
        std::ofstream output(part.file_path, std::ios::binary | std::ios::trunc);
        if (!input || !output)
        {
            m_put_multipart_file_task.reset();
            on_failure(FILE_CREATE_FAILURE, "failed to create multipart part file: " + part.file_path);
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
                m_put_multipart_file_task.reset();
                on_failure(FILE_READ_FAILURE, "failed to read multipart part file: " + part.file_path);
                return;
            }
            output.write(buffer.data(), read_count);
            if (!output)
            {
                m_put_multipart_file_task.reset();
                on_failure(FILE_WRITE_FAILURE, "failed to write multipart part file: " + part.file_path);
                return;
            }
            remaining -= read_count;
        }

        m_put_multipart_file_task->parts.push_back(part);
    }
    if (m_put_multipart_file_task->parts.empty())
    {
        m_put_multipart_file_task.reset();
        return;
    }
    m_put_multipart_file_task->mime_type = mime_type;
    m_put_multipart_file_task->task_info = task_info;
    m_put_multipart_file_task->on_response = on_response;
    m_put_multipart_file_task->on_failure = on_failure;
    put_next_file(0);
}

void INetworkProviderBase::put_next_file(std::size_t index)
{
    const auto& task = m_put_multipart_file_task->parts[index];
    std::map<std::string, std::string> headers;
    headers["Content-Type"] = m_put_multipart_file_task->mime_type;

    put_file(task.url, headers, task.file_path, m_put_multipart_file_task->task_info, [this, index](const amqhi::HttpResponse& response)
    {
        if (response.status_code == 200)
        {
            nlohmann::json composite_result;
            composite_result["part_number"] = m_put_multipart_file_task->parts[index].part_number;
            composite_result["etag"] = response.headers.at("ETag");
            m_put_multipart_file_task->composite_results.push_back(composite_result);
        }
        else
        {
            m_put_multipart_file_task->on_response(response);
            m_put_multipart_file_task.reset();
            return;
        }

        if (index >= m_put_multipart_file_task->parts.size() - 1)
        {
            amqhi::HttpResponse composite_results_response;
            composite_results_response.status_code = 200;
            composite_results_response.body = m_put_multipart_file_task->composite_results.dump();
            m_put_multipart_file_task->on_response(composite_results_response);
            m_put_multipart_file_task.reset();
            return;
        }
        put_next_file(index + 1);
    },
    [this](std::int16_t error_code, const std::string& data)
             {
        m_put_multipart_file_task->on_failure(error_code, data);
            m_put_multipart_file_task.reset();
             });
}


