// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef CLI_FILE_METADATA_H
#define CLI_FILE_METADATA_H
#include <string>

#include "item.h"
#include "sqlite3.h"

struct FileMetadata
{
    std::string checksum;
    std::string mime_type;
    UUID id;
    std::uint64_t size;
};

void cache_file_metadata(sqlite3* db, const FileMetadata& metadata);
void sqlite_bind_file_metadata(sqlite3_stmt* stmt, const FileMetadata& metadata);
void delete_file_metadata(sqlite3* db, const FileMetadata& metadata);
FileMetadata file_metadata_from_stmt(sqlite3_stmt* stmt);
FileMetadata file_metadata_from_json(const nlohmann::json& json);

#endif //CLI_FILE_METADATA_H
