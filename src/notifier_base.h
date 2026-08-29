#ifndef SHARED_NOTIFIER_H
#define SHARED_NOTIFIER_H

#include <string>
#include "json.hpp"

// TODO: standardize event code values and naming
constexpr std::int16_t REGISTER_SUCCESS        = 1000;
constexpr std::int16_t REGISTER_FAILURE        = 1001;
constexpr std::int16_t LOGIN_SUCCESS           = 1002;
constexpr std::int16_t LOGIN_FAILURE           = 1010;
constexpr std::int16_t LOGIN_REQUIRED        = 1011;
constexpr std::int16_t RE_LOGIN_REQUIRED       = 4;
constexpr std::int16_t LOGOUT_SUCCESS          = 5;
constexpr std::int16_t LOGOUT_FAILURE          = 6;
constexpr std::int16_t USER_SWITCHED = 1100;
constexpr std::int16_t REFRESH_SUCCESS           = 7;
constexpr std::int16_t REFRESH_FAILURE           = 27;
constexpr std::int16_t ITEM_CREATE_SUCCESS     = 8;
constexpr std::int16_t ITEM_CREATE_FAILURE     = 9;
constexpr std::int16_t ITEM_UPDATE_SUCCESS            = 10;
constexpr std::int16_t ITEM_UPDATE_FAILURE     = 11;
constexpr std::int16_t ITEM_SOFT_DELETE_SUCCESS            = 13242;
constexpr std::int16_t ITEM_SOFT_DELETE_FAILURE     = 11343;
constexpr std::int16_t ITEM_RESTORE_SUCCESS            = 1077;
constexpr std::int16_t ITEM_RESTORE_FAILURE     = 1177;
constexpr std::int16_t ITEM_DELETED            = 12;
constexpr std::int16_t ITEM_DELETE_FAILURE     = 13;
constexpr std::int16_t ITEM_THUMBNAIL_DOWNLOAD_SUCCESS     = 15;
constexpr std::int16_t ITEM_THUMBNAIL_DOWNLOAD_FAILURE     = 16;
constexpr std::int16_t DATABASE_ERROR          = 17;
constexpr std::int16_t FETCH_USER_INFO_SUCCESS          = 18;
constexpr std::int16_t FETCH_USER_INFO_FAILURE          = 19;
constexpr std::int16_t FILE_DOWNLOAD_SUCCESS = 20;
constexpr std::int16_t FILE_DOWNLOAD_FAILURE        = 21;
constexpr std::int16_t DOWNLOAD_PROGRESS        = 22;
constexpr std::int16_t ITEM_CACHE_SUCCESS = 23;
constexpr std::int16_t ITEM_CACHE_FAILURE        = 24;
constexpr std::int16_t ITEM_MOVE_SUCCESS = 25;
constexpr std::int16_t ITEM_MOVE_FAILURE        = 26;
constexpr std::int16_t UPLOAD_PROGRESS        = 28;
constexpr std::int16_t FETCH_FILE_DOWNLOAD_URL_SUCCESS  = 29;
constexpr std::int16_t FETCH_FILE_DOWNLOAD_URL_FAILURE  = 30;
constexpr std::int16_t NETWORK_CONNECTION_FAILURE = 31;
constexpr std::int16_t SYNC_EVENTS_FETCH_SUCCESS = 32;
constexpr std::int16_t SYNC_EVENTS_FETCH_FAILURE = 33;

class INotifierBase
{
public:
    virtual ~INotifierBase() = default;
    virtual void notify(std::int16_t code) = 0;
    virtual void notify(std::int16_t code, const std::string& data) = 0;
    virtual void notify(std::int16_t code, const nlohmann::json& data) = 0;
};

#endif //SHARED_NOTIFIER_H
