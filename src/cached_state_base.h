
#ifndef SHARED_CORE_CACHED_STATE_BASE_H
#define SHARED_CORE_CACHED_STATE_BASE_H

#include "json.hpp"
#include <string>

#include "date_time_utils.h"
#include "folder_metadata.h"

namespace cached_state_macros
{
#define CACHED_STATE_SETTER(type, var_name) void set_##var_name(type var_name) { \
m_##var_name = var_name; \
}
#define CACHED_STATE_GETTER(type, var_name) [[nodiscard]] type var_name() const { \
return m_##var_name; \
}
}

class CachedStateBase {
public:
    virtual ~CachedStateBase() = default;
    explicit CachedStateBase(const std::string& app_support_path);

    void save();
    void set_selected_user_id(char user_id) {
        m_selected_user_id = user_id;
    }

    [[nodiscard]] char selected_user_id() const
    {
        return m_selected_user_id;
    }

    CACHED_STATE_SETTER(std::int64_t, last_update_check);
    CACHED_STATE_GETTER(std::int64_t, last_update_check);
    void update_last_update_check()
    {
        m_last_update_check = current_date_time_utc_int64();
    }

    CACHED_STATE_SETTER(char, home_folder_view_mode);
    CACHED_STATE_GETTER(char, home_folder_view_mode);
    CACHED_STATE_SETTER(char, home_folder_sort_option);
    CACHED_STATE_GETTER(char, home_folder_sort_option);

    CACHED_STATE_SETTER(char, trash_folder_view_mode);
    CACHED_STATE_GETTER(char, trash_folder_view_mode);
    CACHED_STATE_SETTER(char, trash_folder_sort_option);
    CACHED_STATE_GETTER(char, trash_folder_sort_option);

protected:
    const std::string& m_app_support_path;
    std::int64_t m_last_update_check;
    char m_selected_user_id = 0;
    char m_home_folder_view_mode = view_mode::DEFAULT_MODE;
    char m_home_folder_sort_option = sort_option::DEFAULT_OPTION;
    char m_trash_folder_view_mode = view_mode::DEFAULT_MODE;
    char m_trash_folder_sort_option = sort_option::DEFAULT_OPTION;
    private:
    virtual void bind_data(nlohmann::json& data) = 0;
    virtual void initialize(const nlohmann::json& data) = 0;
};

#endif //SHARED_CORE_CACHED_STATE_BASE_H
