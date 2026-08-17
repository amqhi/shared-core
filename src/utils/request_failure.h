#ifndef SHARED_REQUEST_FAILURE_H
#define SHARED_REQUEST_FAILURE_H

#include <string>

class INotifierBase;
void notify_request_failure(INotifierBase& notifier, int16_t code, int status_code, const std::string& response);
void notify_request_failure(INotifierBase& notifier, int16_t code, int status_code, const std::string& response, const std::string& url);

#endif //SHARED_REQUEST_FAILURE_H
