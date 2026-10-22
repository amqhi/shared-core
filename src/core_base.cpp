
#include "core_base.h"

#include "notifier_base.h"
#include "request_failure.h"
#include "platform.h"
#include "date_time_utils.h"
#include "json_utils.h"
#include "network_error.h"
#include "auth_tokens.h"
#include "network_provider_base.h"
#include "platform_utils_base.h"
#include "secure_storage_provider_base.h"

namespace fs = std::filesystem;

CoreBase::CoreBase(std::string app_support_path, INetworkProviderBase& network_provider,
ISecureStorageProviderBase& secure_storage_provider, INotifierBase& notifier, IPlatformUtilsBase& platform_utils) :
    m_app_support_path(std::move(app_support_path)), m_network_provider(network_provider),
m_secure_storage_provider(secure_storage_provider), m_notifier(notifier), m_platform_utils(platform_utils)
{

}

bool CoreBase::token_refresh_required()
{
    return selected_user()->is_token_refresh_required();
}

User* CoreBase::selected_user()
{
    for (auto& u : m_users)
    {
        if (u.local_id == m_selected_user_id) return &u;
    }
    return nullptr;
}

void CoreBase::destroy() const
{
    m_cached_state->save();
    m_database_provider->close();
}

void CoreBase::on_suspend() const
{
    m_cached_state->save();
}

void CoreBase::on_resume() const
{
    m_item_manager->sync();
}

void CoreBase::add_user()
{
    User user;
    user.local_id = created_user_local_id(m_users);
    m_users.push_back(user);
    m_selected_user_id = user.local_id;
    m_cached_state->set_selected_user_id(m_selected_user_id);
    m_database_provider->close();
    m_item_manager->clear();
    initialize();
}

void CoreBase::switch_user(char user_local_id)
{
    m_selected_user_id = user_local_id;
    m_cached_state->set_selected_user_id(m_selected_user_id);
    m_item_manager->clear();
    m_database_provider->close();
    initialize();
    m_notifier.notify(USER_SWITCHED);
}

void CoreBase::exchange_google_token(const std::string& id_token)
{
    std::string url = m_settings->instance_url() + "/auth/google";
    std::map<std::string, std::string> headers;
    nlohmann::json body;
    body["id_token"] = id_token;
    m_network_provider.post_json(
        url,
        headers,
        body,
        [this](const amqhi::HttpResponse& response)
        {
            if (response.status_code == 200)
            {
                auto json = nlohmann::json::parse(response.body, nullptr, false);
                if (json.is_discarded())
                {
                    selected_user()->access_token = "";
                    selected_user()->refresh_token = "";
                    m_notifier.notify(RE_LOGIN_REQUIRED);
                }
                else
                {
                    AuthTokens auth_tokens = AuthTokens::from_json(json);
                    selected_user()->access_token = auth_tokens.access_token;
                    selected_user()->refresh_token = auth_tokens.refresh_token;
                    selected_user()->access_token_expires_at = User::calculate_expires_at(auth_tokens.expires_in);
                    nlohmann::json data;
                    data["access_token"] = selected_user()->access_token;
                    data["refresh_token"] = selected_user()->refresh_token;
                    data["expires_at"] = selected_user()->access_token_expires_at;
                    m_secure_storage_provider.set_secure_string(m_selected_user_id, data.dump());
                }
            }
            else
            {
            }
        }
        , [this](std::int16_t error_code, const std::string& data)
        {
            handle_network_error(m_notifier, error_code, data);
        });
}

void CoreBase::handle_login(const std::string& instance_url, const std::string& email, const std::string& password)
{
    m_settings->set_instance_url(instance_url);
    m_settings->save();
    handle_login(email, password);
}

void CoreBase::handle_login(const std::string& email, const std::string& password)
{
    std::string url = m_settings->instance_url() + "/auth/login";
    std::map<std::string, std::string> headers;

    nlohmann::json body;
    body["email"] = email;
    body["password"] = password;
    body["device_name"] = m_platform_utils.get_device_name();
    body["device_os"] = platform::name();
    body["app_type"] = "cloud";
    m_network_provider.post_json(
        url,
        headers,
        body,
        [this](const amqhi::HttpResponse& response)
        {
            if (response.status_code == 200)
            {
                auto json = nlohmann::json::parse(response.body, nullptr, false);
                if (json.is_discarded())
                {
                    selected_user()->access_token = "";
                    selected_user()->refresh_token = "";
                    notify_request_failure(m_notifier, LOGIN_FAILURE, response.status_code, response.body);
                }
                else
                {
                    AuthTokens auth_tokens = AuthTokens::from_json(json);
                    selected_user()->access_token = auth_tokens.access_token;
                    selected_user()->refresh_token = auth_tokens.refresh_token;
                    selected_user()->access_token_expires_at = User::calculate_expires_at(auth_tokens.expires_in);
                    nlohmann::json data;
                    data["access_token"] = selected_user()->access_token;
                    data["refresh_token"] = selected_user()->refresh_token;
                    data["expires_at"] = selected_user()->access_token_expires_at;
                    m_secure_storage_provider.set_secure_string(m_selected_user_id, data.dump());
                    m_item_manager->refresh();
                    m_notifier.notify(LOGIN_SUCCESS);
                    fetch_user_info();
                }
            }
            else
            {
                selected_user()->access_token = "";
                selected_user()->refresh_token = "";
                notify_request_failure(m_notifier, LOGIN_FAILURE, response.status_code, response.body);
            }
        }
        , [](std::int16_t error_code, const std::string& data)
        {
        });
}

void CoreBase::fetch_user_info()
{
    std::string url = m_settings->instance_url() + "/users/me";
    std::map<std::string, std::string> headers;
    headers["Authorization"] = "Bearer " + selected_user()->access_token;
    m_network_provider.get(url, headers, [this](const amqhi::HttpResponse& response)
                           {
                               if (response.status_code == 200)
                               {
                                   const nlohmann::json body = nlohmann::json::parse(response.body, nullptr, false);
                                   if (body.is_discarded())
                                   {
                                       notify_request_failure(m_notifier, FETCH_USER_INFO_FAILURE, response.status_code,
                                                              response.body);
                                   }
                                   else
                                   {
                                       selected_user()->name = json_utils::get_string(body, "name");
                                       selected_user()->email = json_utils::get_string(body, "email");
                                       std::string login_type_data = json_utils::get_string(body, "login_type");
                                       if (login_type_data == "google")
                                       {
                                           selected_user()->login_type = login_type::GOOGLE;
                                       }
                                       else
                                       {
                                           selected_user()->login_type = login_type::NORMAL;
                                       }
                                       std::string created_at_data = json_utils::get_string(body, "created_at");
                                       std::string updated_at_data = json_utils::get_string(body, "updated_at");
                                       selected_user()->created_at = parse_iso8601_to_ms(created_at_data);
                                       selected_user()->updated_at = parse_iso8601_to_ms(updated_at_data);

                                       // TODO: Download profile picture
                                       // if (auto it = body.find("profile_picture_id"); it != body.end() && it->is_string())
                                       // {
                                       //     std::string profile_picture_id = it->get<std::string>();
                                       //     api::files::get_download_url(*this, profile_picture_id,
                                       //                                  [this](const amqhi::HttpResponse& response)
                                       //                                  {
                                       //                                      if (response.status_code == 200)
                                       //                                      {
                                       //                                          auto& url = response.body;
                                       //                                          std::map<std::string, std::string> headers;
                                       //                                          m_network_provider.download_file(url, headers, );
                                       //                                      }
                                       //                                      else
                                       //                                      {
                                       //                                      }
                                       //                                  }, [](std::int16_t error_code, const std::string& data)
                                       //                                  {
                                       //                                  });
                                       // }
                                       save_user_data(m_app_support_path, *selected_user());
                                       notifier().notify(FETCH_USER_INFO_SUCCESS);
                                   }
                               }
                               else
                               {
                                   notify_request_failure(m_notifier, FETCH_USER_INFO_FAILURE, response.status_code, response.body);
                               }
                           }, [this](std::int16_t error_code, const std::string& data)
                           {
                               handle_network_error(m_notifier, error_code, data);
                           });
}

void CoreBase::handle_register(const std::string& instance_url, const std::string& email, const std::string& password,
                           const std::string& name) const
{
    m_settings->set_instance_url(instance_url);
    m_settings->save();
    handle_register(email, password, name);
}

void CoreBase::handle_register(const std::string& email, const std::string& password, const std::string& name) const
{
    std::string url = m_settings->instance_url() + "/users";
    std::map<std::string, std::string> headers;
    nlohmann::json body;
    body["email"] = email;
    body["password"] = password;
    body["name"] = name;
    m_network_provider.post_json(
        url,
        headers,
        body,
        [this](const amqhi::HttpResponse& response)
        {
            if (response.status_code == 200)
            {
                m_notifier.notify(REGISTER_SUCCESS);
            }
            else
            {
                notify_request_failure(m_notifier, REGISTER_FAILURE, response.status_code, response.body);
            }
        }
        , [](std::int16_t error_code, const std::string& data)
        {
        });
}

void CoreBase::handle_logout()
{
    std::string url = m_settings->instance_url() + "/auth/logout";
    std::map<std::string, std::string> headers;
    headers["Authorization"] = "Bearer " + selected_user()->access_token;
    nlohmann::json body;
    m_network_provider.post_json(
        url,
        headers,
        body,
        [this](const amqhi::HttpResponse& response)
        {
            if (response.status_code == 200)
            {
                m_secure_storage_provider.remove_secure_string(m_selected_user_id);
                fs::path user_dir_path = fs::path(m_app_support_path) / std::string_view(&m_selected_user_id, 1);
                fs::remove_all(user_dir_path);
                for (auto it = m_users.begin(); it != m_users.end(); ++it)
                {
                    if (it->local_id == m_selected_user_id)
                    {
                        m_users.erase(it);
                        break;
                    }
                }
                if (m_users.empty())
                {
                    User user;
                    user.local_id = created_user_local_id(m_users);
                    m_users.push_back(user);
                    m_selected_user_id = user.local_id;
                }
                else
                {
                    m_selected_user_id = m_users[0].local_id;
                }
                m_database_provider->close();
                m_item_manager->clear();
                m_cached_state->set_selected_user_id(m_selected_user_id);
                initialize();
                m_notifier.notify(LOGOUT_SUCCESS);
            }
            else
            {
                notify_request_failure(m_notifier, LOGOUT_FAILURE, response.status_code, response.body);
            }
        }
        , [this](std::int16_t error_code, const std::string& data)
        {
            handle_network_error(m_notifier, error_code, data);
        });
}
