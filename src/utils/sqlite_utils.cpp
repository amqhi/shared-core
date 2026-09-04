// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include "sqlite_utils.h"

std::string sqlite_utils::get_string(sqlite3_stmt* stmt, int column, const std::string& fallback)
{
    if (sqlite3_column_type(stmt, column) == SQLITE_NULL) {
        return fallback;
    }
    const char* text = reinterpret_cast<const char*>(sqlite3_column_text(stmt, column));
    return text ? text : fallback;
}

std::string sqlite_utils::get_string(sqlite3_stmt* stmt, int column)
{
    return get_string(stmt, column, "");
}

std::optional<std::string> sqlite_utils::get_optional_string(sqlite3_stmt* stmt, int column)
{
    if (sqlite3_column_type(stmt, column) == SQLITE_NULL) {
        return std::nullopt;
    }
    const char* text = reinterpret_cast<const char*>(sqlite3_column_text(stmt, column));
    return text ? std::optional(text) : std::nullopt;
}
