
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef SHARED_CORE_FOLDER_METADATA_H
#define SHARED_CORE_FOLDER_METADATA_H
#include "uuid.h"
#include "json.hpp"
#include "sqlite3.h"

struct FolderMetadata
{
    UUID background_id = special_folder::UNKNOWN;
    UUID icon_id = special_folder::UNKNOWN;
    std::optional<std::uint32_t> background_color = std::nullopt;
    std::optional<std::uint32_t> icon_color = std::nullopt;
};

void cache_folder_metadata(sqlite3* db, const UUID& item_id, const FolderMetadata& metadata);
void sqlite_bind_folder_metadata(sqlite3_stmt* stmt, const UUID& item_id, const FolderMetadata& metadata);
void delete_folder_metadata(sqlite3* db, const UUID& item_id);
FolderMetadata folder_metadata_from_stmt(sqlite3_stmt* stmt);
FolderMetadata folder_metadata_from_json(const nlohmann::json& json);

namespace folder_metadata_column_index {
    constexpr int ID = 0;
    constexpr int BACKGROUND_ID = 1;
    constexpr int BACKGROUND_COLOR = 2;
    constexpr int ICON_ID = 3;
    constexpr int ICON_COLOR = 4;

    namespace bind
    {
        constexpr int ID         = folder_metadata_column_index::ID + 1;
        constexpr int BACKGROUND_ID       = folder_metadata_column_index::BACKGROUND_ID + 1;
        constexpr int BACKGROUND_COLOR       = folder_metadata_column_index::BACKGROUND_COLOR + 1;
        constexpr int ICON_ID       = folder_metadata_column_index::ICON_ID + 1;
        constexpr int ICON_COLOR       = folder_metadata_column_index::ICON_COLOR + 1;
    }
}

#endif //SHARED_CORE_FOLDER_METADATA_H
