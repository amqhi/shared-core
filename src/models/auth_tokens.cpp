// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include "auth_tokens.h"

#include "json_utils.h"

AuthTokens AuthTokens::from_json(const nlohmann::json& json)
{
    AuthTokens auth_tokens;
    auth_tokens.access_token = json_utils::get_string(json, "access_token");
    auth_tokens.refresh_token = json_utils::get_string(json, "refresh_token");
    auth_tokens.expires_in = json_utils::get_int32_t(json, "expires_in");
    // It can be replaced with conditional logic if other token types are added
    auth_tokens.token_type = BEARER;
    return auth_tokens;
}
