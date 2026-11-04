
#include "database_provider_base.h"
#include "core_base.h"

namespace fs = std::filesystem;

void DatabaseProviderBase::initialize_database()
{
    std::filesystem::path user_dir = std::filesystem::path(m_core.app_support_path()) / std::string_view(&m_core.selected_user()->local_id, 1);

    fs::create_directories(user_dir);

    const fs::path db_path = user_dir / database_name();
    sqlite3_open(db_path.string().c_str(), &m_database);

    sqlite3_exec(m_database, "PRAGMA foreign_keys = ON;", nullptr, nullptr, nullptr);

    sqlite3_stmt* stmt = nullptr;
    int current_version = 0;
    if (sqlite3_prepare_v2(m_database, "PRAGMA user_version;", -1, &stmt, nullptr) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            current_version = sqlite3_column_int(stmt, 0);
        }
    }
    sqlite3_finalize(stmt);
    if (current_version == 0)
    {
        int rc = sqlite3_exec(m_database, schema_sql(), nullptr, nullptr, nullptr);
        if (rc == SQLITE_OK)
        {
            sqlite3_exec(m_database, "PRAGMA user_version = 1;", nullptr, nullptr, nullptr);
        }
    }
    else if (current_version != version())
    {
        migrate(current_version);
    }
}

void DatabaseProviderBase::close() const
{
    sqlite3_close(m_database);
}

const char* DatabaseProviderBase::schema_sql()
{
    return R"(
            CREATE TABLE IF NOT EXISTS items (
             id BLOB PRIMARY KEY NOT NULL,
             type INTEGER NOT NULL,
             created_at INTEGER NOT NULL,
             updated_at INTEGER NOT NULL,
             event_at INTEGER,
             deleted_at INTEGER,
             parent_id BLOB,
             name TEXT NOT NULL DEFAULT '',
             comment TEXT,
             icon_type INTEGER NOT NULL DEFAULT 0,
             encrypted INTEGER NOT NULL DEFAULT 0,
             app_scope INTEGER NOT NULL DEFAULT 63,
             status INTEGER NOT NULL,
             FOREIGN KEY(parent_id) REFERENCES items(id) ON DELETE CASCADE
           );

           CREATE INDEX IF NOT EXISTS idx_items_parent_id ON items(parent_id);

           CREATE TABLE IF NOT EXISTS themes (
             id BLOB PRIMARY KEY NOT NULL,
             title TEXT NOT NULL,
             created_at INTEGER NOT NULL,
             updated_at INTEGER NOT NULL,
             data TEXT NOT NULL,
             FOREIGN KEY(id) REFERENCES items(id) ON DELETE CASCADE
           );

           CREATE TABLE IF NOT EXISTS files (
             id BLOB PRIMARY KEY NOT NULL,
             checksum TEXT NOT NULL,
             size INTEGER NOT NULL,
             mime_type TEXT NOT NULL,
             FOREIGN KEY(id) REFERENCES items(id) ON DELETE CASCADE
           );

           CREATE TABLE IF NOT EXISTS folders (
             id BLOB PRIMARY KEY NOT NULL,
             background_id BLOB,
             background_color INTEGER,
             icon_id BLOB,
             icon_color INTEGER,
             view_mode INTEGER NOT NULL DEFAULT 0,
             sort_option INTEGER NOT NULL DEFAULT 0,
             FOREIGN KEY(background_id) REFERENCES items(id) ON DELETE SET NULL,
             FOREIGN KEY(icon_id) REFERENCES items(id) ON DELETE SET NULL,
             FOREIGN KEY(id) REFERENCES items(id) ON DELETE CASCADE
           );
        )";
}
