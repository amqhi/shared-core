#ifndef SHARED_CORE_CORE_BASE_H
#define SHARED_CORE_CORE_BASE_H
#include <string>

#include "cached_state_base.h"
#include "database_provider_base.h"
#include "item_manager_base.h"
#include "settings_base.h"
#include "user.h"

class IPlatformUtilsBase;
class INotifierBase;
class ISecureStorageProviderBase;
class INetworkProviderBase;

class CoreBase
{
public:
    virtual ~CoreBase() = default;
    explicit CoreBase(std::string app_support_path, INetworkProviderBase& network_provider,
                      ISecureStorageProviderBase& secure_storage_provider, INotifierBase& notifier,
                      IPlatformUtilsBase& platform_utils);
    virtual void initialize() = 0;
    [[nodiscard]] bool token_refresh_required();
    void destroy() const;

    void on_suspend() const;
    void on_resume() const;

    void add_user();
    void switch_user(char user_local_id);
    void exchange_google_token(const std::string& id_token);

    /** On first run */
    void handle_login(const std::string& instance_url, const std::string& email, const std::string& password);
    void handle_login(const std::string& email, const std::string& password);
    void fetch_user_info();

    /** On first run */
    void handle_register(const std::string& instance_url, const std::string& email, const std::string& password,
                         const std::string& name) const;
    void handle_register(const std::string& email, const std::string& password, const std::string& name) const;
    void handle_logout();

    [[nodiscard]] INetworkProviderBase& network_provider() const { return m_network_provider; }
    [[nodiscard]] ISecureStorageProviderBase& secure_storage_provider() const { return m_secure_storage_provider; }
    [[nodiscard]] INotifierBase& notifier() const { return m_notifier; }
    [[nodiscard]] IPlatformUtilsBase& platform_utils() const { return m_platform_utils; }
    [[nodiscard]] ItemManagerBase& item_manager() const { return *m_item_manager; }
    [[nodiscard]] SettingsBase& settings() const { return *m_settings; }
    [[nodiscard]] DatabaseProviderBase& database_provider() const { return *m_database_provider; }
    [[nodiscard]] CachedStateBase& cached_state() const { return *m_cached_state; }
    [[nodiscard]] const std::string& app_support_path() const { return m_app_support_path; }

    [[nodiscard]] User* selected_user();
    [[nodiscard]] const std::vector<User>& users() { return m_users; }

protected:
    std::string m_app_support_path;
    INetworkProviderBase& m_network_provider;
    ISecureStorageProviderBase& m_secure_storage_provider;
    INotifierBase& m_notifier;
    IPlatformUtilsBase& m_platform_utils;
    std::unique_ptr<ItemManagerBase> m_item_manager;
    std::unique_ptr<SettingsBase> m_settings;
    std::unique_ptr<CachedStateBase> m_cached_state;
    std::unique_ptr<DatabaseProviderBase> m_database_provider;

    std::vector<User> m_users;

    /** Local ID matching the generated directory name, not the actual user ID. */
    char m_selected_user_id;

private:
    // virtual std::unique_ptr<CachedStateBase> create_cached_state() = 0;
    // virtual std::unique_ptr<SettingsBase> create_settings() = 0;
    // virtual std::unique_ptr<ItemManagerBase> create_item_manager() = 0;
    // virtual std::unique_ptr<INotifierBase> create_notifier() = 0;
};

#endif //SHARED_CORE_CORE_BASE_H
