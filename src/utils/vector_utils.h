// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef SHARED_VECTOR_UTILS_H
#define SHARED_VECTOR_UTILS_H
#include <vector>
#include <unordered_set>

namespace vector_utils
{
    template <typename T>
    void subtract(std::vector<T>& target, const std::vector<T>& to_remove);

    template <typename T>
    void subtract(std::vector<T>& target, const std::vector<T>& to_remove)
    {
        std::unordered_set<T> remove_set(to_remove.begin(), to_remove.end());

        target.erase(
            std::remove_if(target.begin(), target.end(), [&](T x)
            {
    #if __cplusplus >= 202002L
                return remove_set.contains(x);
    #else
                return remove_set.count(x) > 0;
    #endif
            }),
            target.end()
        );
    }
}

#endif //SHARED_VECTOR_UTILS_H
