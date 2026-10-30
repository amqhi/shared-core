
#ifndef SHARED_CORE_SETTINGS_BASE_H
#define SHARED_CORE_SETTINGS_BASE_H

#include <string>
#include "json.hpp"

namespace settings_macros
{
#define SETTINGS_SETTER(type, var_name) void set_##var_name(type var_name) { \
m_##var_name = var_name; \
}
#define SETTINGS_GETTER(type, var_name) [[nodiscard]] type var_name() const { \
    return m_##var_name; \
    }
}

namespace settings_defaults
{
    constexpr const char* const BACKEND_URL = "";
}

class CoreBase;

class SettingsBase
{
public:
    virtual ~SettingsBase() = default;
    explicit SettingsBase(CoreBase& core);
    SETTINGS_SETTER(const std::string&, instance_url)
    SETTINGS_GETTER(const std::string&, instance_url)
    SETTINGS_SETTER(char, android_navigation_bar_style)
    SETTINGS_GETTER(char, android_navigation_bar_style)
    void initialize();
    virtual void initialize(const nlohmann::json& json) = 0;
    void save();

protected:
    CoreBase& m_core;
    std::string m_instance_url;
    char m_android_navigation_bar_style;

private:
    virtual void bind_data(nlohmann::json& json);
};

#endif //SHARED_CORE_SETTINGS_BASE_H
