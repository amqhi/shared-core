// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef SHARED_SECURE_STORAGE_PROVIDER_H
#define SHARED_SECURE_STORAGE_PROVIDER_H

#include <string>

class ISecureStorageProviderBase
{
public:
    virtual ~ISecureStorageProviderBase() = default;
    virtual bool set_secure_string(const std::string_view& key, const std::string& value) = 0;
    virtual std::string get_secure_string(const std::string_view& key, const std::string& default_value) = 0;
    virtual void remove_secure_string(const std::string_view& key) = 0;
    bool set_secure_string(char key, const std::string& value);
    std::string get_secure_string(char key, const std::string& default_value);
    void remove_secure_string(char key);
};

inline void ISecureStorageProviderBase::remove_secure_string(char key)
{
    remove_secure_string(std::string(1, key));
}

inline bool ISecureStorageProviderBase::set_secure_string(char key, const std::string& value)
{
    return set_secure_string(std::string(1, key), value);
}

inline std::string ISecureStorageProviderBase::get_secure_string(char key, const std::string& default_value)
{
    return get_secure_string(std::string(1, key), default_value);
}

#endif //SHARED_SECURE_STORAGE_PROVIDER_H
