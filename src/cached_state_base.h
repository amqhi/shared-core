
#ifndef SHARED_CORE_CACHED_STATE_BASE_H
#define SHARED_CORE_CACHED_STATE_BASE_H

#include "json.hpp"
#include <string>

#include "date_time_utils.h"
#include "item_manager_base.h"
#include "uuid.h"

namespace cached_state_macros
{
#define CACHED_STATE_SETTER(type, var_name) void set_##var_name(type var_name) { \
m_##var_name = var_name; \
}
#define CACHED_STATE_GETTER(type, var_name) [[nodiscard]] type var_name() const { \
return m_##var_name; \
}
}

namespace sort_option
{
    /// A to Z
    constexpr char NAME_ASC = 'n';
    /// Z to A
    constexpr char NAME_DESC = 'N';

    /// Oldest first
    constexpr char CREATED_AT_ASC = 'c';
    /// Newest first
    constexpr char CREATED_AT_DESC = 'C';

    /// Oldest modified first
    constexpr char UPDATED_AT_ASC = 'u';
    /// Recently modified first
    constexpr char UPDATED_AT_DESC = 'U';

    /// Smallest first
    constexpr char SIZE_ASC = 's';
    /// Largest first
    constexpr char SIZE_DESC = 'S';

    /// Type A to Z
    constexpr char TYPE_ASC = 't';
    /// Type Z to A
    constexpr char TYPE_DESC = 'T';

    constexpr char DEFAULT_OPTION = 'U';
}

namespace view_mode
{
    constexpr char GRID = 'g';
    constexpr char LINEAR = 'l';
    constexpr char DEFAULT_MODE = 'g';
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

    void set_sort_option(const UUID& folder_id, char option) {
        m_sort_options[folder_id] = option;
    }

    char sort_option(const UUID& folder_id)
    {
        return m_sort_options[folder_id];
    }

    const std::unordered_map<UUID, char>& sort_options() const
    {
        return m_sort_options;
    }

    void set_view_mode(const UUID& folder_id, char mode)
    {
        m_view_modes[folder_id] = mode;
    }

    char view_mode(const UUID& folder_id)
    {
        return m_view_modes[folder_id];
    }

    const std::unordered_map<UUID, char>& view_modes() const
    {
        return m_view_modes;
    }

    CACHED_STATE_SETTER(std::int64_t, last_update_check);
    CACHED_STATE_GETTER(std::int64_t, last_update_check);
    void update_last_update_check()
    {
        m_last_update_check = current_date_time_utc_int64();
    }

protected:
    std::unordered_map<UUID, char> m_sort_options;
    std::unordered_map<UUID, char> m_view_modes;
    const std::string& m_app_support_path;
    std::int64_t m_last_update_check;
    char m_selected_user_id;
    private:
    virtual void bind_data(nlohmann::json& data) = 0;
    virtual void initialize(const nlohmann::json& data) = 0;
};

#endif //SHARED_CORE_CACHED_STATE_BASE_H
