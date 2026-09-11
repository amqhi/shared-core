// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include "sync_event.h"

#include "date_time_utils.h"
#include "json_utils.h"

SyncEventType string_to_sync_event_type(std::string_view str)
{
    if (str == "create")      return SyncEventType::CREATE;
    if (str == "update")      return SyncEventType::UPDATE;
    if (str == "move")        return SyncEventType::MOVE;
    if (str == "soft_delete") return SyncEventType::SOFT_DELETE;
    if (str == "delete")      return SyncEventType::DELETE;

    return SyncEventType::UNKNOWN;
}
