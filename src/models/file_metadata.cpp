// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include "file_metadata.h"

#include "json_utils.h"

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
    file_metadata.id = UUID::from_bytes(id_bytes);
    file_metadata.checksum = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
    file_metadata.size = sqlite3_column_int64(stmt, 2);
    file_metadata.mime_type = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));

    return file_metadata;
}

FileMetadata file_metadata_from_json(const nlohmann::json& json)
{
    FileMetadata file_metadata;
    file_metadata.id = UUID::from_string(json_utils::get_string(json, "id"));
    file_metadata.checksum = json_utils::get_string(json, "checksum");
    file_metadata.size = json_utils::get_int<std::uint64_t>(json, "size");
    file_metadata.mime_type = json_utils::get_string(json, "mime_type");

    return file_metadata;
}
