// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef SHARED_MAP_UTILS_H
#define SHARED_MAP_UTILS_H

namespace map_utils
{
    template <typename MapType, typename KeyType>
    bool contains_key(const MapType& map, const KeyType& key)
    {
#if __cplusplus >= 202002L
        return map.contains(key);
#else
        return map.find(key) != map.end();
#endif
    }
}

#endif //SHARED_MAP_UTILS_H
