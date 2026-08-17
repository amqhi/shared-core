#ifndef SHARED_SYNC_EVENT_H
#define SHARED_SYNC_EVENT_H
#include <string>

#include "json.hpp"

namespace sync_event_type
{
constexpr const char* CREATE = "create";
constexpr const char* UPDATE = "update";
constexpr const char* MOVE = "move";
constexpr const char* SOFT_DELETE = "soft_delete";
constexpr const char* DELETE = "delete";
}

enum class SyncEventType : std::uint8_t
{
    UNKNOWN = 0,
    CREATE = 1,
    UPDATE = 2,
    MOVE = 3,
    SOFT_DELETE = 4,
    DELETE = 5
};

SyncEventType string_to_sync_event_type(std::string_view str);

struct SyncEvent
{
    std::string id;
    std::string item_id;
    std::int64_t occurred_at;
    SyncEventType type = SyncEventType::UNKNOWN;
};

SyncEvent sync_event_from_json(const nlohmann::json& json);

#endif //SHARED_SYNC_EVENT_H
