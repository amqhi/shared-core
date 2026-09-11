// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef SHARED_COMMON_API_H
#define SHARED_COMMON_API_H
#include <functional>
#include <string>

struct SyncEvent;
class INetworkProviderBase;
using OnResponse = std::function<void(int status_code, const std::string& response)>;
using OnFailure = std::function<void(std::int16_t error_code, const std::string& data)>;

namespace api
{
    namespace sync
    {
        void get_sync_events(const std::string& instance_url, const std::string& access_token, INetworkProviderBase& network_provider_base, const OnResponse& on_response,
                                const OnFailure& on_failure);
        void acknowledge_events(const std::vector<std::string>& item_ids, const std::string& instance_url, const std::string& access_token, INetworkProviderBase& network_provider_base, const OnResponse& on_response, const OnFailure& on_failure);
    }

    namespace items
    {
        void get_item(const std::string& id, const std::string& instance_url, const std::string& access_token, INetworkProviderBase& network_provider_base, const OnResponse& on_response,
                                const OnFailure& on_failure);
    }
}

#endif //SHARED_COMMON_API_H
