// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef SHARED_REQUEST_FAILURE_H
#define SHARED_REQUEST_FAILURE_H

#include <string>

class INotifierBase;
void notify_request_failure(INotifierBase& notifier, int16_t code, int status_code, const std::string& response);
void notify_request_failure(INotifierBase& notifier, int16_t code, int status_code, const std::string& response, const std::string& url);

#endif //SHARED_REQUEST_FAILURE_H
