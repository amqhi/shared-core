
#include "settings_base.h"
#include <fstream>
#include <sstream>

#include "core_base.h"
#include "json_utils.h"

namespace fs = std::filesystem;
using json = nlohmann::json;

SettingsBase::SettingsBase(CoreBase& core) : m_core(core)
{
}

void SettingsBase::initialize()
{
    fs::path file_path = fs::path(m_core.app_support_path()) / std::string_view(& m_core.selected_user()->local_id, 1) / "settings";

    if (!std::filesystem::exists(file_path)) {
        m_instance_url = settings_defaults::BACKEND_URL;
        save();
        return;
    }

    std::ifstream file(file_path);
    if (!file.is_open())
    {
        m_instance_url = settings_defaults::BACKEND_URL;
        return;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();

    auto data = json::parse(content, nullptr, false);

    if (!data.is_discarded())
    {
        m_instance_url = json_utils::get_string(data, "instance_url", settings_defaults::BACKEND_URL);
        m_android_navigation_bar_style = static_cast<char>(json_utils::get_int<std::int8_t>(data, "android_navigation_bar_style", 'n'));
        initialize(data);
    }
}

void SettingsBase::save()
{
    auto user = m_core.selected_user();
    if (!user) return;

    fs::path file_path = fs::path(m_core.app_support_path()) / std::string_view(&user->local_id, 1) / "settings";

    std::error_code ec;
    fs::create_directories(file_path.parent_path(), ec);
    if (ec) return;

    std::ofstream file(file_path);
    if (!file.is_open()) return;

    json data;
    data["instance_url"] = m_instance_url;
    data["android_navigation_bar_style"] = m_android_navigation_bar_style;
    bind_data(data);
    file << data.dump(4) << std::endl;
}

void SettingsBase::bind_data(nlohmann::json& json)
{

}
