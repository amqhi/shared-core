#ifndef SHARED_USER_H
#define SHARED_USER_H
#include <string>
#include "json.hpp"

namespace login_type
{
    constexpr std::int8_t OFFLINE = -1;
    constexpr std::int8_t NORMAL = 0;
    constexpr std::int8_t GOOGLE = 1;
    constexpr std::int8_t APPLE = 2;
    constexpr std::int8_t MICROSOFT = 3;
}

struct User {
    /** Randomly generated directory name for identifying multiple sessions */
    std::string local_id;
    std::string access_token;
    std::string refresh_token;
    std::string name;
    std::string email;
    std::string cached_picture_path;
    std::int64_t expires_at = 0;
    std::int64_t created_at;
    std::int64_t updated_at;
    std::int8_t login_type;
};

void get_user_data(const std::string& app_support_path, User &user);
void save_user_data(const std::string& app_support_path, User &user);

#endif //SHARED_USER_H