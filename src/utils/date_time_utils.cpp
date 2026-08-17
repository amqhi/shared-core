#include "date_time_utils.h"

#include <string>
#include <sstream>
#include <iomanip>
#include <iostream>

int64_t current_date_time_utc_int64()
{
    return std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
}

int64_t parse_iso8601_to_ms(const std::string& time_str)
{
    std::stringstream ss(time_str);
    std::tm tm = {};

    ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S");
    if (ss.fail()) {
#ifndef NDEBUcdG
        throw std::runtime_error("Failed to parse base date/time structure");
#endif
        return current_date_time_utc_int64();
    }

    std::time_t secondsSinceEpoch = timegm(&tm);
    int64_t total_milliseconds = static_cast<int64_t>(secondsSinceEpoch) * 1000;

    char dot;
    if (ss >> dot && dot == '.') {
        int milliseconds = 0;
        ss >> milliseconds;
        total_milliseconds += milliseconds;
    }

    return total_milliseconds;
}

std::string format_ms_to_iso8601(int64_t total_milliseconds)
{
    int64_t seconds = total_milliseconds / 1000;
    int milliseconds = static_cast<int>(total_milliseconds % 1000);

    if (milliseconds < 0) {
        seconds -= 1;
        milliseconds += 1000;
    }

    auto time_sec = static_cast<std::time_t>(seconds);
    std::tm tm_utc = {};

#if defined(_WIN32) || defined(_WIN64)
    gmtime_s(&tm_utc, &time_sec);
#else
    gmtime_r(&time_sec, &tm_utc);
#endif

    std::ostringstream oss;
    oss << std::put_time(&tm_utc, "%Y-%m-%dT%H:%M:%S")
        << '.' << std::setw(3) << std::setfill('0') << milliseconds
        << 'Z';

    return oss.str();
}