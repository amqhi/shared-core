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
        void consume_event(const std::string& event_id, const std::string& instance_url, const std::string& access_token, INetworkProviderBase& network_provider_base, const OnResponse& on_response, const OnFailure& on_failure);
    }

    namespace items
    {
        void get_item(const std::string& id, const std::string& instance_url, const std::string& access_token, INetworkProviderBase& network_provider_base, const OnResponse& on_response,
                                const OnFailure& on_failure);
    }
}

#endif //SHARED_COMMON_API_H
