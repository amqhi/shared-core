
#ifndef SHARED_CORE_JAVA_NETWORK_CALLBACK_H
#define SHARED_CORE_JAVA_NETWORK_CALLBACK_H

#include "network_provider_base.h"

struct CallbackContext {
    std::function<void(const amqhi::HttpResponse&)> response;
    std::function<void(std::int16_t, const std::string&)> failure;
};

#endif //SHARED_CORE_JAVA_NETWORK_CALLBACK_H
