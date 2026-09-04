// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef SHARED_AUTH_TOKENS_H
#define SHARED_AUTH_TOKENS_H
#include <string>

#include "json.hpp"

constexpr int8_t BEARER = 0;

struct AuthTokens {
    std::string access_token;
    std::string refresh_token;
    int32_t expires_at = 0;
    int8_t token_type;

    static AuthTokens from_json(const nlohmann::json& json);
    [[nodiscard]] std::string serialize() const;
};

#endif //SHARED_AUTH_TOKENS_H