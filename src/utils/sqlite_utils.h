#ifndef SHARED_SQLITE_UTILS_H
#define SHARED_SQLITE_UTILS_H
#include <string>
#include "sqlite3.h"

namespace sqlite_utils
{
    std::string get_string(sqlite3_stmt* stmt, int column, const std::string& fallback);
    std::string get_string(sqlite3_stmt* stmt, int column);
    std::optional<std::string> get_optional_string(sqlite3_stmt* stmt, int column);
}

#endif //SHARED_SQLITE_UTILS_H
