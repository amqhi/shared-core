// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef SHARED_NETWORK_ERROR_H
#define SHARED_NETWORK_ERROR_H
#include <string>

class INotifierBase;
void handle_network_error(INotifierBase& notifier, int16_t error_code, const std::string& data);

#endif //SHARED_NETWORK_ERROR_H
