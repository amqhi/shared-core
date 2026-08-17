#ifndef SHARED_SECURE_STORAGE_PROVIDER_H
#define SHARED_SECURE_STORAGE_PROVIDER_H

#include <string>

class ISecureStorageProviderBase
{
public:
    virtual ~ISecureStorageProviderBase() = default;
    virtual bool set_secure_string(const std::string& key, const std::string& value) = 0;
    virtual std::string get_secure_string(const std::string& key, const std::string& default_value) = 0;
    virtual void remove_secure_string(const std::string& key) = 0;
};

#endif //SHARED_SECURE_STORAGE_PROVIDER_H
