// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef SHARED_COMMON_API_H
#define SHARED_COMMON_API_H
#include <functional>
#include <string>

#include "auth_tokens.h"

struct FileMetadata;
class CoreBase;
struct UUID;
struct SyncEvent;
class INetworkProviderBase;
using OnResponse = std::function<void(int status_code, const std::string& response)>;
using OnFailure = std::function<void(std::int16_t error_code, const std::string& data)>;

namespace api
{
    void request_with_auto_refresh(
        CoreBase& core,
        const std::function<void()>& execute_request
    );

    namespace auth
    {
        void refresh(
            CoreBase& core,
            const std::function<void()>& on_success);
    }

    namespace sync
    {
        void get_sync_events_raw(
            CoreBase& core,
            const OnResponse& on_response,
            const OnFailure& on_failure);
        void get_sync_events(
            CoreBase& core,
            const OnResponse& on_response,
            const OnFailure& on_failure);
        void acknowledge_events_raw(
            CoreBase& core,
            const std::vector<std::string>& item_ids,
            const OnResponse& on_response,
            const OnFailure& on_failure);
        void acknowledge_events(
            CoreBase& core,
            const std::vector<std::string>& item_ids,
            const OnResponse& on_response,
            const OnFailure& on_failure);
    }

    namespace items
    {
        void get_item_raw(
            CoreBase& core,
            const std::string& id,
            const OnResponse& on_response,
            const OnFailure& on_failure);
        void get_item(
            CoreBase& core,
            const std::string& id,
            const OnResponse& on_response,
            const OnFailure& on_failure);
        void get_thumbnail_download_url_raw(
            CoreBase& core,
            const std::string& id,
            const OnResponse& on_response,
            const OnFailure& on_failure);
        void get_thumbnail_download_url(
            CoreBase& core,
            const std::string& id,
            const OnResponse& on_response,
            const OnFailure& on_failure);

        void get_thumbnail_download_url(
            CoreBase& core,
            const std::string& id,
            const OnResponse& on_response);

        void move_items_raw(
            CoreBase& core,
            const std::vector<std::string>& ids,
            const std::string& parent_id,
            const OnResponse& on_response,
            const OnFailure& on_failure);
        void move_items(
            CoreBase& core,
            const std::vector<std::string>& ids,
            const std::string& parent_id,
            const OnResponse& on_response,
            const OnFailure& on_failure);
        void move_items(
              CoreBase& core,
              const std::vector<UUID>& ids,
              const UUID& parent_id,
              const OnResponse& on_response,
              const OnFailure& on_failure);

        void soft_delete_items_raw(
            CoreBase& core,
            const std::vector<std::string>& ids,
            const OnResponse& on_response,
            const OnFailure& on_failure);
        void soft_delete_items(
            CoreBase& core,
            const std::vector<std::string>& ids,
            const OnResponse& on_response,
            const OnFailure& on_failure);
        void soft_delete_items(
            CoreBase& core,
            const std::vector<UUID>& ids,
            const OnResponse& on_response,
            const OnFailure& on_failure);

        void restore_items_raw(
            CoreBase& core,
            const std::vector<std::string>& ids,
            const OnResponse& on_response,
            const OnFailure& on_failure);
        void restore_items(
            CoreBase& core,
            const std::vector<std::string>& ids,
            const OnResponse& on_response,
            const OnFailure& on_failure);
        void restore_items(
            CoreBase& core,
            const std::vector<UUID>& ids,
            const OnResponse& on_response,
            const OnFailure& on_failure);

        void delete_items_raw(
            CoreBase& core,
            const std::vector<std::string>& ids,
            const OnResponse& on_response,
            const OnFailure& on_failure);
        void delete_items(
            CoreBase& core,
            const std::vector<std::string>& ids,
            const OnResponse& on_response,
            const OnFailure& on_failure);
        void delete_items(
            CoreBase& core,
            const std::vector<UUID>& ids,
            const OnResponse& on_response,
            const OnFailure& on_failure);
    }

    namespace files
    {
        void get_files_raw(CoreBase& core, const OnResponse& on_response,
                           const OnFailure& on_failure);
        void get_files(CoreBase& core, const OnResponse& on_response,
                       const OnFailure& on_failure);
        void get_file_metadata_raw(
            CoreBase& core,
            const std::string& id,
            const OnResponse& on_response,
            const OnFailure& on_failure
        );
        void get_file_metadata(
            CoreBase& core,
            const std::string& id,
            const OnResponse& on_response,
            const OnFailure& on_failure
        );
        void get_file_metadata(
            CoreBase& core,
            const std::string& id,
            const OnResponse& on_response
        );
        void get_file_metadata(
            CoreBase& core,
            const std::string& id,
            const std::function<void(FileMetadata& file_metadata)>& on_success
        );
        void get_download_url_raw(
            CoreBase& core,
            const std::string& id,
            const std::function<void(int status_code, const std::string& response)>& on_response,
            const std::function<void(std::int16_t error_code, const std::string& data)>& on_failure
        );
        void get_download_url(
            CoreBase& core,
            const std::string& id,
            const std::function<void(int status_code, const std::string& response)>& on_response,
            const std::function<void(std::int16_t error_code, const std::string& data)>& on_failure
        );
    }

    namespace folders
    {
        void get_folders_raw(CoreBase& core, const OnResponse& on_response,
                             const OnFailure& on_failure);
        void get_folders(CoreBase& core, const OnResponse& on_response,
                         const OnFailure& on_failure);
    }
}

#endif //SHARED_COMMON_API_H
