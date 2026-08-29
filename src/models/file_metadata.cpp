#include "file_metadata.h"

void cache_file_metadata(sqlite3* db, const FileMetadata& metadata)
{
    if (db == nullptr) {
        return;
    }

    const char* sql = R"(INSERT INTO files (id, checksum, size, mime_type)
VALUES (?, ?, ?, ?)
ON CONFLICT(id) DO UPDATE SET
  checksum = excluded.checksum,
  size = excluded.size,
  mime_type = excluded.mime_type;)";

    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return;
    }

    sqlite_bind_file_metadata(stmt, metadata);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

void sqlite_bind_file_metadata(sqlite3_stmt* stmt, const FileMetadata& file_metadata)
{
    auto id_bytes = file_metadata.id.to_bytes();
    sqlite3_bind_blob(stmt, 1, &id_bytes, id_bytes.size(), SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, file_metadata.checksum.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int64(stmt, 3, static_cast<sqlite3_int64>(file_metadata.size));
    sqlite3_bind_text(stmt, 4, file_metadata.mime_type.c_str(), -1, SQLITE_TRANSIENT);
}

void delete_file_metadata(sqlite3* db, const FileMetadata& metadata)
{
    if (db == nullptr) {
        return;
    }

    const char* sql = "DELETE FROM files WHERE id = ?;";

    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return;
    }

    auto id_bytes = metadata.id.to_bytes();
    sqlite3_bind_blob(stmt, 1, &id_bytes, id_bytes.size(), SQLITE_STATIC);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

FileMetadata file_metadata_from_stmt(sqlite3_stmt* stmt)
{
    FileMetadata file_metadata;
    const void* id_bytes = sqlite3_column_blob(stmt, 0);
    file_metadata.id = ItemId::from_bytes(id_bytes);
    file_metadata.checksum = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
    file_metadata.size = sqlite3_column_int64(stmt, 2);
    file_metadata.mime_type = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));

    return file_metadata;
}
