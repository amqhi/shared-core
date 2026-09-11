// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef SHARED_SYNC_EVENT_H
#define SHARED_SYNC_EVENT_H
#include <string>

#include "json.hpp"

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

#endif //SHARED_SYNC_EVENT_H
