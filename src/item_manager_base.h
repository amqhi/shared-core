
#ifndef SHARED_CORE_ITEM_MANAGER_BASE_H
#define SHARED_CORE_ITEM_MANAGER_BASE_H

#include "item.h"
#include <functional>

#include "file_metadata.h"
#include "folder_metadata.h"
#include "json.hpp"

#include "item_attributes.h"
#include "network_provider_base.h"

class CoreBase;
class INetworkProvider;
class INotifier;
struct User;

class ItemManagerBase
{
    public:
    virtual ~ItemManagerBase() = default;
    explicit ItemManagerBase(CoreBase& core);
    const std::vector<UUID>& id_list_by_id(const UUID& parent_id);
    [[nodiscard]] const Item& item_by_id(const UUID& id);
    [[nodiscard]] const FileMetadata& file_metadata_by_id(const UUID& id);
    [[nodiscard]] const FolderMetadata& folder_metadata_by_id(const UUID& id);
    virtual void clear();
    virtual void initialize();
    virtual void sync();
    virtual void refresh();
    void sort_items(char option, const UUID& parent_id);
    void sort_items(const UUID& parent_id);
    void create_file(const ItemAttributes& item_attributes, const std::string& tmp_file_path);
    void create_folder(const ItemAttributes& item_attributes);
    void complete_upload_multipart(const Item& item, const std::string& checksum, std::uint64_t size,
                                   const std::string& mime_type, const std::string& upload_id, const
                                   nlohmann::json& parts);
    void complete_upload(const Item& item, const std::string& checksum, std::uint64_t size,
                         const std::string& mime_type);
    void update_item(const UUID& id, const ItemAttributes& item_attributes);
    void move_item(const UUID& id, const UUID& parent_id);
    void move_items(const std::vector<UUID>& ids, const UUID& parent_id, const UUID& old_parent_id);
    void rename_item(const UUID& id, const std::string& name);
    void soft_delete_item(const UUID& id);
    void soft_delete_items(const std::vector<UUID>& ids, const UUID& old_parent_id);
    void restore_item(const UUID& id);
    void restore_items(const std::vector<UUID>& ids, const UUID& old_parent_id);
    void delete_item(const UUID& id);
    void delete_items(const std::vector<UUID>& ids);
    void download_thumbnails(std::int16_t event_code);
    void download_thumbnail(const UUID& id);
    void make_item_available_offline(const UUID& id);
    void download_item(const UUID& id, const std::string& file_path);
    void download_item(const UUID& id, const std::string& file_path,
                       const std::function<void(const amqhi::HttpResponse& response)>& on_response);
    void fetch_file_download_url(const UUID& id) const;

protected:
    CoreBase& m_core;

    std::unordered_map<UUID, Item> m_items;
    std::unordered_map<UUID, std::vector<UUID>> m_id_lists;
    std::unordered_map<UUID, FileMetadata> m_file_metadata;
    std::unordered_map<UUID, FolderMetadata> m_folder_metadata;

private:

    void refresh_phase1_files();
    void refresh_phase2_folders();

    // Prefix 'apply_' indicates mutating internal state (m_items, m_id_lists)
    void apply_create_item(const Item& item);
    void apply_update_item(const Item& item);
    void apply_move_item(const UUID& id, const UUID& old_parent_id, const UUID& parent_id);
    void apply_delete_item(const UUID& id, const UUID& parent_id);

};


#endif //SHARED_CORE_ITEM_MANAGER_BASE_H
