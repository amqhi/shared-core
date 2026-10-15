// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include "common_api.h"

#include <map>

#include "core_base.h"
#include "json_utils.h"
#include "network_error.h"
#include "network_provider_base.h"
#include "uuid.h"

void api::sync::get_sync_events(const std::string& instance_url, const std::string& access_token, INetworkProviderBase& network_provider_base, const OnResponse& on_response,
                                const OnFailure& on_failure)
{
    std::string url = instance_url + "/sync/events";
    std::map<std::string, std::string> headers;
    headers["Authorization"] = "Bearer " + access_token;
    network_provider_base.get(url,
                              headers,
                              on_response, on_failure);
}

void api::sync::get_sync_events(CoreBase& core, const OnResponse& on_response, const OnFailure& on_failure)
{
    get_sync_events(core.settings().instance_url(), core.selected_user()->access_token, core.network_provider(), on_response, on_failure);
}

void api::sync::acknowledge_events(const std::vector<std::string>& item_ids, const std::string& instance_url,
    const std::string& access_token, INetworkProviderBase& network_provider_base, const OnResponse& on_response,
    const OnFailure& on_failure)
{
    std::string url = instance_url + "/sync/events/acknowledge";
    std::map<std::string, std::string> headers;
    headers["Authorization"] = "Bearer " + access_token;
    nlohmann::json body;
    body["item_ids"] = item_ids;
    network_provider_base.post_json(url, headers, body, on_response, on_failure);
}

void api::items::get_item(const std::string& id, const std::string& instance_url, const std::string& access_token,
                          INetworkProviderBase& network_provider_base, const OnResponse& on_response, const OnFailure& on_failure)
{
    std::string url = instance_url + "/items/" + id;
    std::map<std::string, std::string> headers;
    headers["Authorization"] = "Bearer " + access_token;
    network_provider_base.get(url,
                              headers,
                              on_response, on_failure);
}

void api::items::get_thumbnail_download_url(const UUID& item_id, const std::string& instance_url,
    const std::string& access_token, INetworkProviderBase& network_provider_base, const OnResponse& on_response,
    const OnFailure& on_failure)
{
    std::string url = instance_url + "/items/" + item_id.to_string() + "/thumbnail/download-url";
    std::map<std::string, std::string> headers;
    headers["Authorization"] = "Bearer " + access_token;
    network_provider_base.get(url,
                                  headers,
                                  on_response, on_failure);
}

void api::files::get_files(const std::string& instance_url, const std::string& access_token,
                           INetworkProviderBase& network_provider_base, const OnResponse& on_response, const OnFailure& on_failure)
{
    std::string url = instance_url + "/files";
    std::map<std::string, std::string> headers;
    headers["Authorization"] = "Bearer " + access_token;
    network_provider_base.get(url,
                              headers,
                              on_response, on_failure);
}

void api::folders::get_folders(const std::string& instance_url, const std::string& access_token,
                               INetworkProviderBase& network_provider_base, const OnResponse& on_response, const OnFailure& on_failure)
{
    std::string url = instance_url + "/folders";
    std::map<std::string, std::string> headers;
    headers["Authorization"] = "Bearer " + access_token;
    network_provider_base.get(url,
                              headers,
                              on_response, on_failure);
}

void api::files::get_file_metadata(CoreBase& core, const std::string& id, const OnResponse& on_response,
                                   const OnFailure& on_failure)
{
    std::string url = core.settings().instance_url() + "/files/" + id;
    std::map<std::string, std::string> headers;
    headers["Authorization"] = "Bearer " + core.selected_user()->access_token;
    core.network_provider().get(url,
                                headers,
                                on_response,
                                on_failure);
}

void api::files::get_file_metadata(CoreBase& core, const std::string& id, const OnResponse& on_response)
{
    get_file_metadata(core, id, on_response, [&core](std::int16_t error_code, const std::string& data)
    {
        handle_network_error(core.notifier(), error_code, data);
    });
}

void api::files::get_file_metadata(CoreBase& core, const std::string& id,
                                   const std::function<void(FileMetadata& file_metadata)>& on_success)
{
    get_file_metadata(core, id, [on_success](int status_code, const std::string& response)
    {
        if (status_code == 200)
        {
            auto body = nlohmann::json::parse(response, nullptr, false);
            if (!body.is_discarded())
            {
                FileMetadata file_metadata;
                file_metadata.id = UUID::from_string(json_utils::get_string(body, "id"));
                file_metadata.checksum = json_utils::get_string(body, "checksum");
                file_metadata.size = json_utils::get_int64_t(body, "size");
                file_metadata.mime_type = json_utils::get_string(body, "mime_type");
                on_success(file_metadata);
            }
        }
    });
}

void api::files::get_download_url(CoreBase& core, const std::string& id,
                                  const std::function<void(int status_code, const std::string& response)>& on_response,
                                  const std::function<void(std::int16_t error_code, const std::string& data)>& on_failure)
{
    std::string url = core.settings().instance_url() + "/files/" + id + "/download-url";
    std::map<std::string, std::string> headers;
    headers["Authorization"] = "Bearer " + core.selected_user()->access_token;
    core.network_provider().get(url,
                                headers,
                                on_response,
                                on_failure);
}

void api::items::get_item(CoreBase& core, const std::string& id, const OnResponse& on_response, const OnFailure& on_failure)
{
    get_item(id, core.settings().instance_url(), core.selected_user()->access_token,core.network_provider(), on_response, on_failure);
}

void api::items::get_thumbnail_download_url(CoreBase& core, const std::string& id, const OnResponse& on_response,
                                            const OnFailure& on_failure)
{
    std::string url = core.settings().instance_url() + "/items/" + id + "/thumbnail";
    std::map<std::string, std::string> headers;
    headers["Authorization"] = "Bearer " + core.selected_user()->access_token;
    core.network_provider().get(url,
                                headers,
                                on_response, on_failure);
}

void api::items::get_thumbnail_download_url(CoreBase& core, const std::string& id, const OnResponse& on_response)
{
}

void api::items::get_thumbnail_download_url(CoreBase& core, const std::string& id,
                                            const std::function<void(std::string& url)>& on_success)
{
}

void api::sync::acknowledge_events(CoreBase& core, const std::vector<std::string>& item_ids, const OnResponse& on_response,
    const OnFailure& on_failure)
{
    acknowledge_events(item_ids, core.settings().instance_url(), core.selected_user()->access_token, core.network_provider(), on_response, on_failure);
}
