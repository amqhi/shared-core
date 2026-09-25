
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef SHARED_CORE_FOLDER_METADATA_H
#define SHARED_CORE_FOLDER_METADATA_H
#include "item.h"

struct FolderMetadata
{
    UUID background_id = special_folder::UNKNOWN;
    UUID icon_id = special_folder::UNKNOWN;
    std::optional<std::uint32_t> background_color = std::nullopt;
    std::optional<std::uint32_t> icon_color = std::nullopt;
};

#endif //SHARED_CORE_FOLDER_METADATA_H
