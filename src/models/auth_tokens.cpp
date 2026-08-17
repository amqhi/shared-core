#include "auth_tokens.h"

#include "json_utils.h"

AuthTokens AuthTokens::from_json(const nlohmann::json& json)
{
    AuthTokens auth_tokens;
    auth_tokens.access_token = json_utils::get_string(json, "access_token");
    auth_tokens.refresh_token = json_utils::get_string(json, "refresh_token");
    auth_tokens.expires_at = json_utils::get_int32_t(json, "expires_at");
    // It can be replaced with conditional logic if other token types are added
    auth_tokens.token_type = BEARER;
    return auth_tokens;
}

std::string AuthTokens::serialize() const
{
    auto json = nlohmann::json();
    json["access_token"] = access_token;
    json["refresh_token"] = refresh_token;
    json["expires_at"] = expires_at;
    json["token_type"] = token_type;
    return to_string(json);
}
