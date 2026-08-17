#ifndef SHARED_DATE_TIME_UTILS_H
#define SHARED_DATE_TIME_UTILS_H

#include <chrono>

int64_t current_date_time_utc_int64();

int64_t parse_iso8601_to_ms(const std::string& time_str);

std::string format_ms_to_iso8601(int64_t total_milliseconds);

#endif //SHARED_DATE_TIME_UTILS_H
