#ifndef RETINFO_H
#define RETINFO_H


#include <json/json.h>
#include <mutex>
#include <condition_variable>

void syncHandler(void * obj, int retCode, const Json::Value &payload,const Json::Value &extraInfo);

using Ms = std::chrono::milliseconds;

struct sRetInfo
{
    sRetInfo()
    {
        // Write notification...
        written = false;
    }

    ~sRetInfo()
    {
    }

    void setWritten()
    {
        std::unique_lock<std::mutex> lock(mutex_waitWritten);
        written = true;
        cond_waitWritten.notify_all();
    }
    bool waitUntilWritten(uint32_t msecs)
    {
        std::unique_lock<std::mutex> lock(mutex_waitWritten);
        if (written)
            return true;
        return cond_waitWritten.wait_for(lock,Ms(msecs)) != std::cv_status::timeout;
    }

    int retCode;
    Json::Value payload, extraInfo;

    // Notification mechanism...
    bool written;
    std::mutex mutex_waitWritten;
    std::condition_variable cond_waitWritten;
};


#endif // RETINFO_H
