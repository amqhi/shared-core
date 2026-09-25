// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include "folder_metadata.h"


void cache_folder_metadata(sqlite3* db, const UUID& item_id, const FolderMetadata& metadata)
{
    if (db == nullptr) {
        return;
    }
    const char* sql = R"(INSERT INTO folders (id, background_id, background_color, icon_id, icon_color)
VALUES (?, ?, ?, ?, ?)
ON CONFLICT(id) DO UPDATE SET
  background_id = excluded.background_id,
  background_color = excluded.background_color,
  icon_id = excluded.icon_id,
  icon_color = excluded.icon_color;)";

    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return;
    }

    sqlite_bind_folder_metadata(stmt, item_id, metadata);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

void sqlite_bind_folder_metadata(sqlite3_stmt* stmt, const UUID& item_id, const FolderMetadata& metadata)
{
    auto id_bytes = item_id.to_bytes();
    sqlite3_bind_blob(stmt, folder_metadata_column_index::ID, &id_bytes, id_bytes.size(), SQLITE_TRANSIENT);
    if (metadata.background_id == special_folder::UNKNOWN)
    {
        sqlite3_bind_null(stmt, folder_metadata_column_index::BACKGROUND_ID);
    }
    else
    {
        auto background_id_bytes = metadata.background_id.to_bytes();
        sqlite3_bind_blob(stmt, folder_metadata_column_index::BACKGROUND_ID, &background_id_bytes, background_id_bytes.size(), SQLITE_TRANSIENT);
    }
    if (metadata.background_color.has_value())
    {
        sqlite3_bind_int(stmt, folder_metadata_column_index::BACKGROUND_COLOR, static_cast<int>(metadata.background_color.value()));
    }
    else
    {
        sqlite3_bind_null(stmt, folder_metadata_column_index::BACKGROUND_COLOR);
    }

    if (metadata.icon_id == special_folder::UNKNOWN)
    {
        sqlite3_bind_null(stmt, folder_metadata_column_index::ICON_ID);
    }
    else
    {
        auto icon_id_bytes = metadata.icon_id.to_bytes();
        sqlite3_bind_blob(stmt, folder_metadata_column_index::ICON_ID, &icon_id_bytes, icon_id_bytes.size(), SQLITE_TRANSIENT);
    }
    if (metadata.icon_color.has_value())
    {
        sqlite3_bind_int(stmt, folder_metadata_column_index::ICON_COLOR, static_cast<int>(metadata.icon_color.value()));
    }
    else
    {
        sqlite3_bind_null(stmt, folder_metadata_column_index::ICON_COLOR);
    }

}

void delete_folder_metadata(sqlite3* db, const UUID& item_id)
{
    if (db == nullptr) {
        return;
    }

    const char* sql = "DELETE FROM folders WHERE id = ?;";

    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return;
    }

    auto id_bytes = item_id.to_bytes();
    sqlite3_bind_blob(stmt, 1, &id_bytes, id_bytes.size(), SQLITE_STATIC);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

FolderMetadata folder_metadata_from_stmt(sqlite3_stmt* stmt)
{
    FolderMetadata folder_metadata;

    if (sqlite3_column_type(stmt, folder_metadata_column_index::BACKGROUND_ID) == SQLITE_NULL)
    {
        const void* background_id_bytes = sqlite3_column_blob(stmt, folder_metadata_column_index::BACKGROUND_ID);
        folder_metadata.background_id = UUID::from_bytes(background_id_bytes);
    }
    else
    {
        folder_metadata.background_id = special_folder::UNKNOWN;
    }
    if (sqlite3_column_type(stmt, folder_metadata_column_index::BACKGROUND_COLOR) == SQLITE_NULL)
    {
        folder_metadata.background_color = std::nullopt;
    }
    else
    {
        folder_metadata.background_color = sqlite3_column_int(stmt, folder_metadata_column_index::BACKGROUND_COLOR);
    }
    if (sqlite3_column_type(stmt, folder_metadata_column_index::ICON_ID) == SQLITE_NULL)
    {
        const void* icon_id_bytes = sqlite3_column_blob(stmt, folder_metadata_column_index::ICON_ID);
        folder_metadata.icon_id = UUID::from_bytes(icon_id_bytes);
    }
    else
    {
        folder_metadata.icon_id = special_folder::UNKNOWN;
    }
    if (sqlite3_column_type(stmt, folder_metadata_column_index::ICON_COLOR) == SQLITE_NULL)
    {
        folder_metadata.icon_color = std::nullopt;
    }
    else
    {
        folder_metadata.icon_color = sqlite3_column_int(stmt, folder_metadata_column_index::ICON_COLOR);
    }
    return folder_metadata;
}

FolderMetadata folder_metadata_from_json(const nlohmann::json& json)
{
    FolderMetadata folder_metadata;
    if (auto it = json.find("background_id"); it != json.end() && it->is_string())
    {
        folder_metadata.background_id = UUID::from_string(it->get<std::string>());
    }
    else
    {
        folder_metadata.background_id = special_folder::UNKNOWN;
    }
    if (auto it = json.find("background_color"); it != json.end() && it->is_string())
    {
        // TODO: Parse hex string color
        folder_metadata.background_color = 0;
    }
    else
    {
        folder_metadata.background_color = std::nullopt;
    }

    if (auto it = json.find("icon_id"); it != json.end() && it->is_string())
    {
        folder_metadata.icon_id = UUID::from_string(it->get<std::string>());
    }
    else
    {
        folder_metadata.icon_id = special_folder::UNKNOWN;
    }
    if (auto it = json.find("icon_color"); it != json.end() && it->is_string())
    {
        // TODO: Parse hex string color
        folder_metadata.icon_color = 0;
    }
    else
    {
        folder_metadata.icon_color = std::nullopt;
    }
    return folder_metadata;
}
