#ifndef SHARED_NETWORK_ERROR_H
#define SHARED_NETWORK_ERROR_H
#include <string>

class INotifierBase;
void handle_network_error(INotifierBase& notifier, int16_t error_code, const std::string& data);

#endif //SHARED_NETWORK_ERROR_H
