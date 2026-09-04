// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef SHARED_DATE_TIME_UTILS_H
#define SHARED_DATE_TIME_UTILS_H

#include <chrono>

int64_t current_date_time_utc_int64();

int64_t parse_iso8601_to_ms(const std::string& time_str);

std::string format_ms_to_iso8601(int64_t total_milliseconds);

#endif //SHARED_DATE_TIME_UTILS_H
