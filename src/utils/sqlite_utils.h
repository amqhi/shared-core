// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

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

struct Sqlite3Stmt
{
    sqlite3_stmt* stmt = nullptr;

    Sqlite3Stmt() = default;
    explicit Sqlite3Stmt(sqlite3_stmt* s) : stmt(s) {}

    int bind(int index, const void* ptr, int size) const
    {
        return bind(index, ptr, size, SQLITE_TRANSIENT);
    }

    int bind(int index, const void* ptr, int size, sqlite3_destructor_type destructor) const
    {
        return sqlite3_bind_blob(stmt, index, ptr, size, destructor);
    }

    int prepare(
    sqlite3 *db,            /* Database handle */
const char *zSql,       /* SQL statement, UTF-8 encoded */
int nByte,              /* Maximum length of zSql in bytes. */
const char **pzTail     /* OUT: Pointer to unused portion of zSql */
)
    {
        return sqlite3_prepare_v2(db, zSql, nByte, &stmt, pzTail);
    }

    int prepare(sqlite3 *db, const char *zSql)
    {
        return prepare(db, zSql, -1, nullptr);
    }

    [[nodiscard]] int step() const
    {
        return sqlite3_step(stmt);
    }

    ~Sqlite3Stmt()
    {
        sqlite3_finalize(stmt);
    }
};

#endif //SHARED_SQLITE_UTILS_H
