#ifndef GarbageCollector_H
#define GarbageCollector_H

#include "threaded.h"

#include <condition_variable>
#include <mutex>

class GarbageCollector
{
public:
    GarbageCollector(const uint32_t &periodMS = 3000);
    void startGC(void (*gcRunner)(void *obj), void *obj);

    virtual ~GarbageCollector();

    void loopGC();
    void setGCPeriodMS(const uint32_t & msecs);

private:
    int gcThreadStatus;

    std::mutex mutex_endNotificationLoop;
    std::condition_variable cond_endNotification;

    void (*gcRunner)(void *obj);
    void * obj;

    bool gcFinished;
    std::thread xThreadGC;
    std::atomic<uint32_t> gcPeriodMS;
};


#endif // GarbageCollector_H
