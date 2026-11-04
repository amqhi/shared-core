
#ifndef SHARED_CORE_DATABASE_PROVIDER_BASE_H
#define SHARED_CORE_DATABASE_PROVIDER_BASE_H

#include <filesystem>

#include "sqlite3.h"

class CoreBase;

class DatabaseProviderBase
{
public:
    virtual ~DatabaseProviderBase() = default;
    explicit DatabaseProviderBase(CoreBase& core) : m_core(core) {}
    void initialize_database();
    virtual int version() = 0;
    void close() const;
    [[nodiscard]] sqlite3* database() const
    {
        return m_database;
    }
protected:
    CoreBase& m_core;
    sqlite3* m_database = nullptr;

    virtual void migrate(int old_version) const = 0;

    virtual std::filesystem::path database_name() = 0;
    virtual const char* schema_sql();
};

#endif //SHARED_CORE_DATABASE_PROVIDER_BASE_H
