// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include "request_failure.h"
#include "json.hpp"
#include "notifier_base.h"

void notify_request_failure(INotifierBase& notifier, int16_t code, int status_code, const std::string& response)
{
    nlohmann::json data;
    data["status_code"] = status_code;
    data["response"] = response;
    notifier.notify(code, data);
}

void notify_request_failure(INotifierBase& notifier, int16_t code, int status_code, const std::string& response,
                            const std::string& url)
{
    nlohmann::json data;
    data["status_code"] = status_code;
    data["response"] = response;
    data["url"] = url;
    notifier.notify(code, data);
}
