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

void api::sync::consume_event(const std::string& event_id, const std::string& instance_url,
    const std::string& access_token, INetworkProviderBase& network_provider_base, const OnResponse& on_response,
    const OnFailure& on_failure)
{
    std::string url = instance_url + "/sync/events/" + event_id + "/consume";
    std::map<std::string, std::string> headers;
    headers["Authorization"] = "Bearer " + access_token;
    network_provider_base.post(url, headers, on_response, on_failure);
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
