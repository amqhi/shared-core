#include "user.h"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <chrono>

#include "date_time_utils.h"
#include "json_utils.h"

namespace fs = std::filesystem;

void get_user_data(const std::string& app_support_path, User& user)
{
    fs::path file_path = fs::path(app_support_path) / user.local_id / "profile";
    if (!std::filesystem::exists(file_path)) {
        return;
    }
    std::ifstream file(file_path);

    if (!file.is_open()) return;

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();

    auto data = nlohmann::json::parse(content, nullptr, false);
    if (!data.is_discarded())
    {
        user.name = json_utils::get_string(data, "name");
        user.email = json_utils::get_string(data, "email");
        std::string login_type_data = json_utils::get_string(data, "login_type");
        if (login_type_data == "google")
        {
            user.login_type = login_type::GOOGLE;
        }
        else
        {
            user.login_type = login_type::NORMAL;
        }

        std::string created_at_data = json_utils::get_string(data, "created_at");
        user.created_at = parse_iso8601_to_ms(created_at_data);
        std::string updated_at_data = json_utils::get_string(data, "updated_at");
        user.updated_at = parse_iso8601_to_ms(updated_at_data);
        // TODO: Get local profile picture path
    }
}

void save_user_data(const std::string& app_support_path, User& user)
{
    fs::path file_path = fs::path(app_support_path) / user.local_id / "profile";
    if (!fs::exists(file_path.parent_path())) {
        fs::create_directories(file_path.parent_path());
    }

    std::ofstream file(file_path);

    nlohmann::json data;

    data["name"] = user.name;
    data["email"] = user.email;
    if (user.login_type == login_type::GOOGLE)
    {
        data["login_type"] = "google";
    }
    else
    {
        data["login_type"] = "normal";
    }
    data["created_at"] = format_ms_to_iso8601(user.created_at);
    data["updated_at"] = format_ms_to_iso8601(user.updated_at);

    if (file.is_open() && file.good()) {
        file << data.dump(4) << std::endl;
    }
    file.close();
}
