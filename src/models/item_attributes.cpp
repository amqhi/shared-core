// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include "item_attributes.h"

#include "item.h"

void to_json(nlohmann::json& j, const ItemAttributes& attributes) {
    j = nlohmann::json{
            {"name", attributes.name}
    };

    if (attributes.parent_id != special_folder::HOME && attributes.parent_id != special_folder::TRASH)
    {
        j["parent_id"] = attributes.parent_id.to_string();
    }

    if (attributes.comment.has_value())
    {
        j["comment"] = attributes.comment.value();
    }

    if (attributes.event_at.has_value())
    {
        j["event_at"] = attributes.event_at.value();
    }

    j["app_scope"] = app_scope_to_vector(attributes.app_scope);

    j["encrypted"] = attributes.encrypted;
}
