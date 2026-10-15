// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include "common_api.h"

#include <map>

#include "core_base.h"
#include "json_utils.h"
#include "network_error.h"
#include "network_provider_base.h"
#include "uuid.h"
#include "notifier_base.h"
#include "secure_storage_provider_base.h"
//
// void api::request_with_auto_refresh(CoreBase& core, const std::function<void()>& execute_request)
// {
//     if (core.selected_user()->is_token_refresh_required())
//     {
//         auth::refresh(core, [&execute_request]()
//         {
//             execute_request();
//         });
//         return;
//     }
//     execute_request();
// }


void api::sync::get_sync_events(CoreBase& core, const OnResponse& on_response, const OnFailure& on_failure)
{
        std::string url = core.settings().instance_url() + "/sync/events";
        std::map<std::string, std::string> headers;
        headers["Authorization"] = "Bearer " + core.selected_user()->access_token;
        core.network_provider().get(url,
                                  headers,
                                  on_response, on_failure);
}

void api::sync::acknowledge_events(
            CoreBase& core,
            const std::vector<std::string>& item_ids,
            const OnResponse& on_response,
            const OnFailure& on_failure)
{
    std::string url = core.settings().instance_url() + "/sync/events/acknowledge";
    std::map<std::string, std::string> headers;
    headers["Authorization"] = "Bearer " + core.selected_user()->access_token;
    nlohmann::json body;
    body["item_ids"] = item_ids;
    core.network_provider().post_json(url, headers, body, on_response, on_failure);
}

void api::items::get_item(CoreBase& core,
            const std::string& id, const OnResponse& on_response,
                          const OnFailure& on_failure)
{
    std::string url = core.settings().instance_url() + "/items/" + id;
    std::map<std::string, std::string> headers;
    headers["Authorization"] = "Bearer " + core.selected_user()->access_token;
    core.network_provider().get(url,
                              headers,
                              on_response, on_failure);
}

void api::items::get_thumbnail_download_url(
            CoreBase& core,
            const std::string& id,
            const OnResponse& on_response,
            const OnFailure& on_failure)
{
    std::string url = core.settings().instance_url() + "/items/" + id + "/thumbnail/download-url";
    std::map<std::string, std::string> headers;
    headers["Authorization"] = "Bearer " + core.selected_user()->access_token;
    core.network_provider().get(url,
                              headers,
                              on_response, on_failure);
}

void api::files::get_files(CoreBase& core, const OnResponse& on_response,
                           const OnFailure& on_failure)
{
    std::string url = core.settings().instance_url() + "/files";
    std::map<std::string, std::string> headers;
    headers["Authorization"] = "Bearer " + core.selected_user()->access_token;
    core.network_provider().get(url,
                              headers,
                              on_response, on_failure);
}

void api::folders::get_folders(CoreBase& core, const OnResponse& on_response,
                               const OnFailure& on_failure)
{
    std::string url = core.settings().instance_url() + "/folders";
    std::map<std::string, std::string> headers;
    headers["Authorization"] = "Bearer " + core.selected_user()->access_token;
    core.network_provider().get(url,
                              headers,
                              on_response, on_failure);
}

void api::files::get_file_metadata(CoreBase& core, const std::string& id, const OnResponse& on_response,
                                   const OnFailure& on_failure)
{
    std::string url = core.settings().instance_url() + "/files/" + id;
    std::map<std::string, std::string> headers;
    headers["Authorization"] = "Bearer " + core.selected_user()->access_token;
    core.network_provider().get(url,
                                headers,
                                on_response,
                                on_failure);
}

void api::files::get_file_metadata(CoreBase& core, const std::string& id, const OnResponse& on_response)
{
    get_file_metadata(core, id, on_response, [&core](std::int16_t error_code, const std::string& data)
    {
        handle_network_error(core.notifier(), error_code, data);
    });
}

void api::files::get_file_metadata(CoreBase& core, const std::string& id,
                                   const std::function<void(FileMetadata& file_metadata)>& on_success)
{
    get_file_metadata(core, id, [on_success](int status_code, const std::string& response)
    {
        if (status_code == 200)
        {
            auto body = nlohmann::json::parse(response, nullptr, false);
            if (!body.is_discarded())
            {
                FileMetadata file_metadata;
                file_metadata.id = UUID::from_string(json_utils::get_string(body, "id"));
                file_metadata.checksum = json_utils::get_string(body, "checksum");
                file_metadata.size = json_utils::get_int64_t(body, "size");
                file_metadata.mime_type = json_utils::get_string(body, "mime_type");
                on_success(file_metadata);
            }
        }
    });
}

void api::files::get_download_url(CoreBase& core, const std::string& id,
                                  const std::function<void(int status_code, const std::string& response)>& on_response,
                                  const std::function<void(std::int16_t error_code, const std::string& data)>&
                                  on_failure)
{
    std::string url = core.settings().instance_url() + "/files/" + id + "/download-url";
    std::map<std::string, std::string> headers;
    headers["Authorization"] = "Bearer " + core.selected_user()->access_token;
    core.network_provider().get(url,
                                headers,
                                on_response,
                                on_failure);
}

void api::items::get_thumbnail_download_url(CoreBase& core, const std::string& id, const OnResponse& on_response)
{
    get_thumbnail_download_url(core, id, on_response, [](std::int16_t error_code, const std::string& data)
    {

    });
}

// void api::auth::refresh(CoreBase& core, const std::function<void()>& on_success)
// {
//     std::string url = core.settings().instance_url() + "/auth/refresh";
//     std::map<std::string, std::string> headers;
//     nlohmann::json body;
//     body["refresh_token"] = core.selected_user()->refresh_token;
//     core.network_provider().post_json(
//         url,
//         headers,
//         body,
//         [&core, &on_success](int status_code, const std::string& response)
//         {
//             if (status_code == 200)
//             {
//                 auto json = nlohmann::json::parse(response, nullptr, false);
//                 if (json.is_discarded())
//                 {
//                     core.selected_user()->access_token = "";
//                     core.selected_user()->refresh_token = "";
//                     core.notifier().notify(RE_LOGIN_REQUIRED);
//                 }
//                 else
//                 {
//                     AuthTokens auth_tokens = AuthTokens::from_json(json);
//                     core.secure_storage_provider().set_secure_string(
//                         std::string_view(&core.selected_user()->local_id, 1),
//                         auth_tokens.serialize());
//                     core.selected_user()->access_token = auth_tokens.access_token;
//                     core.selected_user()->refresh_token = auth_tokens.refresh_token;
//                     core.selected_user()->access_token_expires_at = User::calculate_expires_at(auth_tokens.expires_in);
//                     on_success();
//                 }
//             }
//             else
//             {
//                 core.selected_user()->access_token = "";
//                 core.selected_user()->refresh_token = "";
//                 core.secure_storage_provider().remove_secure_string(
//                     std::string_view(&core.selected_user()->local_id, 1));
//                 core.notifier().notify(RE_LOGIN_REQUIRED);
//             }
//         }
//         , [&core](std::int16_t error_code, const std::string& data)
//         {
//             handle_network_error(core.notifier(), error_code, data);
//         });
// }
