#ifndef UMAMUSUMELOCALIFYANDROID_NOTIFIER_H
#define UMAMUSUMELOCALIFYANDROID_NOTIFIER_H

#pragma once
#include <string>

namespace notifier
{
    void init();

    void notify_response(const std::string& data);

    void notify_request(const std::string& data);
}

#endif //UMAMUSUMELOCALIFYANDROID_NOTIFIER_H
