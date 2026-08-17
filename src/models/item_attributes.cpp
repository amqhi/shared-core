#include "item_attributes.h"

#include "item.h"

void to_json(nlohmann::json& j, const ItemAttributes& attributes) {
    j = nlohmann::json{
            {"name", attributes.name}
    };

    if (!attributes.parent_id.empty() && attributes.parent_id != special_folder::HOME && attributes.parent_id != special_folder::TRASH)
    {
        j["parent_id"] = attributes.parent_id;
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
