// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef SHARED_ITEM_ATTRIBUTES_H
#define SHARED_ITEM_ATTRIBUTES_H

#include <string>
#include <vector>
#include <optional>

#include "item.h"
#include "json.hpp"

struct ItemAttributes {
    std::string name;
    UUID parent_id = special_folder::HOME;
    std::optional<std::string> comment = std::nullopt;
    std::optional<std::int64_t> event_at = std::nullopt;
    
    /**
   * Bitmask representing which applications can view/access this item.
   * Default value is 63 (0b111111: all scopes enabled).
   */
    std::int16_t app_scope = 63;
    bool encrypted = false;
};

void to_json(nlohmann::json& j, const ItemAttributes& attributes);

#endif //SHARED_ITEM_ATTRIBUTES_H
