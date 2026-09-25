include(FetchContent)

if(CMAKE_CXX_STANDARD GREATER_EQUAL 20)
    set(JSON_URL "https://github.com/nlohmann/json/releases/download/v3.11.3/json.hpp")
    set(SQLITE_URL "https://www.sqlite.org/2024/sqlite-amalgamation-3450100.zip")
else()
    # For lower version C++
    set(JSON_URL "https://github.com/nlohmann/json/releases/download/v3.10.5/json.hpp")
    set(SQLITE_URL "https://www.sqlite.org/2023/sqlite-amalgamation-3410200.zip")
endif()


set(JSON_DEST_DIR "${CMAKE_CURRENT_LIST_DIR}/deps/json")
set(JSON_HEADER "${JSON_DEST_DIR}/json.hpp")

if(NOT EXISTS ${JSON_HEADER})
    message(STATUS "Downloading json.hpp for C++${CMAKE_CXX_STANDARD}...")
    file(DOWNLOAD ${JSON_URL} ${JSON_HEADER}
            SHOW_PROGRESS
            STATUS JSON_DOWNLOAD_STATUS
    )
    list(GET JSON_DOWNLOAD_STATUS 0 JSON_ERR)
    if(NOT JSON_ERR EQUAL 0)
        message(FATAL_ERROR "Failed to download json.hpp: ${JSON_DOWNLOAD_STATUS}")
    endif()
endif()



set(SQLITE_DEST_DIR "${CMAKE_CURRENT_LIST_DIR}/deps/sqlite")
set(SQLITE_C_FILE "${SQLITE_DEST_DIR}/sqlite3.c")

if(NOT EXISTS ${SQLITE_C_FILE})
    message(STATUS "Downloading SQLite amalgamation...")
    set(SQLITE_ZIP "${CMAKE_BINARY_DIR}/sqlite_temp.zip")

    file(DOWNLOAD ${SQLITE_URL} ${SQLITE_ZIP} SHOW_PROGRESS)

    file(ARCHIVE_EXTRACT INPUT ${SQLITE_ZIP} DESTINATION "${CMAKE_BINARY_DIR}/sqlite_tmp")

    file(GLOB EXTRACTED_SQLITE_DIR "${CMAKE_BINARY_DIR}/sqlite_tmp/sqlite-amalgamation-*")
    file(COPY "${EXTRACTED_SQLITE_DIR}/sqlite3.c" "${EXTRACTED_SQLITE_DIR}/sqlite3.h"
            DESTINATION ${SQLITE_DEST_DIR})

    file(REMOVE_RECURSE ${SQLITE_ZIP} "${CMAKE_BINARY_DIR}/sqlite_tmp")
endif()

set(SHARED_INCLUDE_DIRS
        ${CMAKE_CURRENT_LIST_DIR}/src
        ${CMAKE_CURRENT_LIST_DIR}/src/utils
        ${CMAKE_CURRENT_LIST_DIR}/src/models
        ${CMAKE_CURRENT_LIST_DIR}/deps
        ${CMAKE_CURRENT_LIST_DIR}/deps/sqlite
        ${CMAKE_CURRENT_LIST_DIR}/deps/json
)

set(SHARED_SOURCES
        ${CMAKE_CURRENT_LIST_DIR}/deps/sqlite/sqlite3.c
        ${CMAKE_CURRENT_LIST_DIR}/deps/sqlite/sqlite3.h
        ${CMAKE_CURRENT_LIST_DIR}/deps/json/json.hpp
        ${CMAKE_CURRENT_LIST_DIR}/src/utils/map_utils.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/utils/map_utils.h
        ${CMAKE_CURRENT_LIST_DIR}/src/utils/vector_utils.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/utils/vector_utils.h
        ${CMAKE_CURRENT_LIST_DIR}/src/utils/sqlite_utils.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/utils/sqlite_utils.h
        ${CMAKE_CURRENT_LIST_DIR}/src/utils/json_utils.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/utils/json_utils.h
        ${CMAKE_CURRENT_LIST_DIR}/src/notifier_base.h
        ${CMAKE_CURRENT_LIST_DIR}/src/network_provider_base.h
        ${CMAKE_CURRENT_LIST_DIR}/src/secure_storage_provider_base.h
        ${CMAKE_CURRENT_LIST_DIR}/src/platform_utils_base.h
        ${CMAKE_CURRENT_LIST_DIR}/src/utils/date_time_utils.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/utils/date_time_utils.h
        ${CMAKE_CURRENT_LIST_DIR}/src/utils/mime_utils.h
        ${CMAKE_CURRENT_LIST_DIR}/src/utils/name_generator.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/utils/name_generator.h
        ${CMAKE_CURRENT_LIST_DIR}/src/utils/network_error.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/utils/network_error.h
        ${CMAKE_CURRENT_LIST_DIR}/src/utils/request_failure.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/utils/request_failure.h
        ${CMAKE_CURRENT_LIST_DIR}/src/models/user.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/models/user.h
        ${CMAKE_CURRENT_LIST_DIR}/src/models/item.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/models/item.h
        ${CMAKE_CURRENT_LIST_DIR}/src/models/auth_tokens.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/models/auth_tokens.h
        ${CMAKE_CURRENT_LIST_DIR}/src/models/item_attributes.h
        ${CMAKE_CURRENT_LIST_DIR}/src/models/item_attributes.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/common_api.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/common_api.h
        ${CMAKE_CURRENT_LIST_DIR}/src/models/sync_event.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/models/sync_event.h
        ${CMAKE_CURRENT_LIST_DIR}/src/models/file_metadata.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/models/file_metadata.h
        ${CMAKE_CURRENT_LIST_DIR}/src/models/folder_metadata.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/models/folder_metadata.h
        ${CMAKE_CURRENT_LIST_DIR}/src/models/uuid.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/models/uuid.h
)

set(SHARED_JAVA_BINDING_INCLUDE_DIRS
        ${CMAKE_CURRENT_LIST_DIR}/bindings/java
)

set(SHARED_JAVA_BINDING_SOURCES
        ${CMAKE_CURRENT_LIST_DIR}/bindings/java/java_string.h
        ${CMAKE_CURRENT_LIST_DIR}/bindings/java/java_map.h
        ${CMAKE_CURRENT_LIST_DIR}/bindings/java/java_network_provider_base.h
        ${CMAKE_CURRENT_LIST_DIR}/bindings/java/java_notifier_base.h
        ${CMAKE_CURRENT_LIST_DIR}/bindings/java/java_secure_storage_provider_base.h
        ${CMAKE_CURRENT_LIST_DIR}/bindings/java/java_platform_utils_base.h
        ${CMAKE_CURRENT_LIST_DIR}/bindings/java/java_list.h
        ${CMAKE_CURRENT_LIST_DIR}/bindings/java/java_list.cpp
        ${CMAKE_CURRENT_LIST_DIR}/bindings/java/java_item.h
)