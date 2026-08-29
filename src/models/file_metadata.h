#ifndef CLI_FILE_METADATA_H
#define CLI_FILE_METADATA_H
#include <string>

#include "item.h"
#include "sqlite3.h"

struct FileMetadata
{
    std::string checksum;
    std::string mime_type;
    ItemId id;
    std::uint64_t size;
};

void cache_file_metadata(sqlite3* db, const FileMetadata& metadata);
void sqlite_bind_file_metadata(sqlite3_stmt* stmt, const FileMetadata& metadata);
void delete_file_metadata(sqlite3* db, const FileMetadata& metadata);
FileMetadata file_metadata_from_stmt(sqlite3_stmt* stmt);

#endif //CLI_FILE_METADATA_H
