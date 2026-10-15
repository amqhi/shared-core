
#include "cached_state_base.h"
#include <fstream>
#include <filesystem>
#include <sstream>

namespace fs = std::filesystem;
using json = nlohmann::json;

CachedStateBase::CachedStateBase(const std::string& app_support_path) : m_app_support_path(app_support_path)
{
    fs::path file_path = fs::path(app_support_path) / "state";

    if (!std::filesystem::exists(file_path)) {
        return;
    }

    std::ifstream file(file_path);
    if (!file.is_open()) return;

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();

    auto data = json::parse(content, nullptr, false);
    if (data.is_discarded())
    {
        save();
        return;
    }

    if (auto it = data.find("selected_user_id"); it != data.end() && it->is_number_integer())
    {
        m_selected_user_id = it->get<char>();
    }
}

void CachedStateBase::save()
{
    fs::path file_path = fs::path(m_app_support_path) / "state";

    if (!fs::exists(file_path.parent_path())) {
        fs::create_directories(file_path.parent_path());
    }

    std::ofstream file(file_path);

    json data;

    data["selected_user_id"] = m_selected_user_id;
    auto& sort_options_json = data["sort_options"] = nlohmann::json::object();
    for (const auto& pair : m_sort_options)
    {
        sort_options_json[pair.first.to_string()] = pair.second;
    }

    auto& view_modes_json = data["view_modes"] = nlohmann::json::object();
    for (const auto& pair : m_view_modes)
    {
        view_modes_json[pair.first.to_string()] = pair.second;
    }
    bind_data(data);

    if (file.is_open() && file.good()) {
        file << data.dump(4) << std::endl;
    }
    file.close();
}
