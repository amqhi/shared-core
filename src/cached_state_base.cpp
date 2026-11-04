
#include "cached_state_base.h"
#include <fstream>
#include <filesystem>
#include <sstream>

#include "json_utils.h"

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

    m_last_update_check = json_utils::get_int<std::int64_t>(data, "last_update_check");

    if (auto it = data.find("home_folder_view_mode"); it != data.end() && it->is_number_integer())
    {
        m_home_folder_view_mode = it->get<char>();
    }
    if (auto it = data.find("home_folder_sort_option"); it != data.end() && it->is_number_integer())
    {
        m_home_folder_sort_option = it->get<char>();
    }
    if (auto it = data.find("trash_folder_view_mode"); it != data.end() && it->is_number_integer())
    {
        m_trash_folder_view_mode = it->get<char>();
    }
    if (auto it = data.find("trash_folder_sort_option"); it != data.end() && it->is_number_integer())
    {
        m_trash_folder_sort_option = it->get<char>();
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

    data["last_update_check"] = m_last_update_check;

    data["home_folder_view_mode"] = m_home_folder_view_mode;
    data["home_folder_sort_option"] = m_home_folder_sort_option;
    data["trash_folder_view_mode"] = m_trash_folder_view_mode;
    data["trash_folder_sort_option"] = m_trash_folder_sort_option;

    bind_data(data);

    if (file.is_open() && file.good()) {
        file << data.dump(4) << std::endl;
    }
    file.close();
}
