
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef SHARED_CORE_FOLDER_METADATA_H
#define SHARED_CORE_FOLDER_METADATA_H
#include "uuid.h"
#include "json.hpp"
#include "sqlite3.h"

namespace sort_option
{
    /// A to Z
    constexpr char NAME_ASC = 'n';
    /// Z to A
    constexpr char NAME_DESC = 'N';

    /// Oldest first
    constexpr char CREATED_AT_ASC = 'c';
    /// Newest first
    constexpr char CREATED_AT_DESC = 'C';

    /// Oldest modified first
    constexpr char UPDATED_AT_ASC = 'u';
    /// Recently modified first
    constexpr char UPDATED_AT_DESC = 'U';

    /// Smallest first
    constexpr char SIZE_ASC = 's';
    /// Largest first
    constexpr char SIZE_DESC = 'S';

    /// Type A to Z
    constexpr char TYPE_ASC = 't';
    /// Type Z to A
    constexpr char TYPE_DESC = 'T';

    constexpr char DEFAULT_OPTION = 'U';
}

namespace view_mode
{
    constexpr char GRID = 'g';
    constexpr char LINEAR = 'l';
    constexpr char DEFAULT_MODE = 'g';
}


struct FolderMetadata
{
    amqhi::UUID background_id = special_folder::UNKNOWN;
    amqhi::UUID icon_id = special_folder::UNKNOWN;
    std::optional<std::uint32_t> background_color = std::nullopt;
    std::optional<std::uint32_t> icon_color = std::nullopt;
    char view_mode;
    char sort_option;
};

void cache_folder_metadata(sqlite3* db, const amqhi::UUID& item_id, const FolderMetadata& metadata);
void sqlite_bind_folder_metadata(sqlite3_stmt* stmt, const amqhi::UUID& item_id, const FolderMetadata& metadata);
void delete_folder_metadata(sqlite3* db, const amqhi::UUID& item_id);
FolderMetadata folder_metadata_from_stmt(sqlite3_stmt* stmt);
FolderMetadata folder_metadata_from_json(const nlohmann::json& json);

namespace folder_metadata_column_index {
    constexpr int ID = 0;
    constexpr int BACKGROUND_ID = 1;
    constexpr int BACKGROUND_COLOR = 2;
    constexpr int ICON_ID = 3;
    constexpr int ICON_COLOR = 4;
    constexpr int VIEW_MODE = 5;
    constexpr int SORT_OPTION = 6;

    namespace bind
    {
        constexpr int ID         = folder_metadata_column_index::ID + 1;
        constexpr int BACKGROUND_ID       = folder_metadata_column_index::BACKGROUND_ID + 1;
        constexpr int BACKGROUND_COLOR       = folder_metadata_column_index::BACKGROUND_COLOR + 1;
        constexpr int ICON_ID       = folder_metadata_column_index::ICON_ID + 1;
        constexpr int ICON_COLOR       = folder_metadata_column_index::ICON_COLOR + 1;
        constexpr int VIEW_MODE = folder_metadata_column_index::VIEW_MODE + 1;
        constexpr int SORT_OPTION = folder_metadata_column_index::SORT_OPTION + 1;
    }
}

#endif //SHARED_CORE_FOLDER_METADATA_H
