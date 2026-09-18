// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef SHARED_NOTIFIER_H
#define SHARED_NOTIFIER_H

#include <string>
#include "json.hpp"

// -----------------------------------------------------------------------------
// System & Auth (1000 ~ 1099)
// -----------------------------------------------------------------------------
constexpr std::int16_t REGISTER_SUCCESS               = 1000;
constexpr std::int16_t REGISTER_FAILURE               = 1001;
constexpr std::int16_t LOGIN_SUCCESS                  = 1002;
constexpr std::int16_t LOGIN_FAILURE                  = 1003;
constexpr std::int16_t LOGIN_REQUIRED                 = 1004;
constexpr std::int16_t RE_LOGIN_REQUIRED              = 1005;
constexpr std::int16_t LOGOUT_SUCCESS                 = 1006;
constexpr std::int16_t LOGOUT_FAILURE                 = 1007;
constexpr std::int16_t USER_SWITCHED                  = 1008;
constexpr std::int16_t REFRESH_SUCCESS                = 1009;
constexpr std::int16_t REFRESH_FAILURE                = 1010;
constexpr std::int16_t FETCH_USER_INFO_SUCCESS         = 1011;
constexpr std::int16_t FETCH_USER_INFO_FAILURE         = 1012;

// -----------------------------------------------------------------------------
// Item CRUD & Actions (1100 ~ 1199)
// -----------------------------------------------------------------------------
constexpr std::int16_t ITEM_CREATE_SUCCESS            = 1100;
constexpr std::int16_t ITEM_CREATE_FAILURE            = 1101;
constexpr std::int16_t ITEM_UPDATE_SUCCESS            = 1102;
constexpr std::int16_t ITEM_UPDATE_FAILURE            = 1103;
constexpr std::int16_t ITEM_DELETED                   = 1104;
constexpr std::int16_t ITEM_DELETE_FAILURE            = 1105;
constexpr std::int16_t ITEM_SOFT_DELETE_SUCCESS       = 1106;
constexpr std::int16_t ITEM_SOFT_DELETE_FAILURE       = 1107;
constexpr std::int16_t ITEM_RESTORE_SUCCESS           = 1108;
constexpr std::int16_t ITEM_RESTORE_FAILURE           = 1109;
constexpr std::int16_t ITEM_MOVE_SUCCESS              = 1110;
constexpr std::int16_t ITEM_MOVE_FAILURE              = 1111;
constexpr std::int16_t ITEM_CACHE_SUCCESS             = 1112;
constexpr std::int16_t ITEM_CACHE_FAILURE             = 1113;

// -----------------------------------------------------------------------------
// File & Transfer (1200 ~ 1299)
// -----------------------------------------------------------------------------
constexpr std::int16_t FILE_DOWNLOAD_SUCCESS          = 1200;
constexpr std::int16_t FILE_DOWNLOAD_FAILURE          = 1201;
constexpr std::int16_t DOWNLOAD_PROGRESS               = 1202;
constexpr std::int16_t UPLOAD_PROGRESS                 = 1203;
constexpr std::int16_t ITEM_THUMBNAIL_DOWNLOAD_SUCCESS  = 1204;
constexpr std::int16_t ITEM_THUMBNAIL_DOWNLOAD_FAILURE  = 1205;
constexpr std::int16_t FETCH_FILE_DOWNLOAD_URL_SUCCESS = 1206;
constexpr std::int16_t FETCH_FILE_DOWNLOAD_URL_FAILURE = 1207;
constexpr std::int16_t FETCH_FILES_FAILURE            = 1208;
constexpr std::int16_t FETCH_FOLDERS_FAILURE          = 1209;

// -----------------------------------------------------------------------------
// Sync (1300 ~ 1399)
// -----------------------------------------------------------------------------
constexpr std::int16_t SYNC_STARTED                   = 1300;
constexpr std::int16_t SYNC_SUCCESS                   = 1301;
constexpr std::int16_t SYNC_EVENTS_FETCH_FAILURE      = 1302;
constexpr std::int16_t ACKNOWLEDGE_SYNC_EVENTS_FETCH_FAILURE = 1303;

// -----------------------------------------------------------------------------
// General Errors (1900 ~ 1999)
// -----------------------------------------------------------------------------
constexpr std::int16_t DATABASE_ERROR                 = 1900;
constexpr std::int16_t NETWORK_CONNECTION_FAILURE    = 1901;

class INotifierBase
{
public:
    virtual ~INotifierBase() = default;
    virtual void notify(std::int16_t code) = 0;
    virtual void notify(std::int16_t code, const std::string& data) = 0;
    virtual void notify(std::int16_t code, const nlohmann::json& data) = 0;
};

#endif //SHARED_NOTIFIER_H
