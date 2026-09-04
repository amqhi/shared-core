
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef SHARED_PLATFORM_H
#define SHARED_PLATFORM_H
#include <string_view>

#if defined(_WIN32) || defined(_WIN64)
    #define PLATFORM_WINDOWS 1
#elif defined(__APPLE__) && defined(__MACH__)
    #include <TargetConditionals.h>
    #if TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR
        #define PLATFORM_IOS 1
    #else
        #define PLATFORM_MACOS 1
    #endif
    #define PLATFORM_APPLE 1
#elif defined(__ANDROID__)
    #define PLATFORM_ANDROID 1   // Must come BEFORE __linux__
#elif defined(__linux__)
    #define PLATFORM_LINUX 1
#elif defined(__FreeBSD__) || defined(__OpenBSD__)
    #define PLATFORM_BSD 1
#else
    #define PLATFORM_UNKNOWN 1
#endif

namespace platform {

    constexpr std::string_view name() noexcept {
#if PLATFORM_WINDOWS
        return "windows";
#elif PLATFORM_MACOS
        return "macos";
#elif PLATFORM_IOS
        return "ios";
#elif PLATFORM_ANDROID
        return "android";
#elif PLATFORM_LINUX
        return "linux";
#else
        return "unknown";
#endif
    }

}

#endif //SHARED_PLATFORM_H
