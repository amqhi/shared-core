#include "network_error.h"

#include "notifier_base.h"

// Notify network request failure to UI/CLI (e.g., connection failure, timeout, etc.)
void handle_network_error(INotifierBase& notifier, std::int16_t error_code, const std::string& data)
{
    notifier.notify(error_code, data);
}
