// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include "common_api.h"

#include <map>

#include "network_provider_base.h"

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
