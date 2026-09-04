// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include "network_error.h"

#include "notifier_base.h"

// Notify network request failure to UI/CLI (e.g., connection failure, timeout, etc.)
void handle_network_error(INotifierBase& notifier, std::int16_t error_code, const std::string& data)
{
    notifier.notify(error_code, data);
}
