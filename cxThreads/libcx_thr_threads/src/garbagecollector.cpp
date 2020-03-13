#include "garbagecollector.h"
#include <errno.h>
#include <iostream>

/*
#include <thread>
#include <mutex>
#include <iostream>*/

using Ms = std::chrono::milliseconds;

void threadExecuter_GCLoop(GarbageCollector * threadClass)
{
    threadClass->loopGC();
}

GarbageCollector::GarbageCollector(const uint32_t &periodMS)
{
  //  std::cout << "setting GC to " << &gcRunner << std::endl << std::flush;
    gcFinished = false;
    gcThreadStatus = -1;
    gcPeriodMS = periodMS;
}

void GarbageCollector::startGC(void (*gcRunner)(void *), void *obj)
{
    this->gcRunner = gcRunner;
    this->obj = obj;
    xThreadGC = std::thread(threadExecuter_GCLoop,this);
}

GarbageCollector::~GarbageCollector()
{
    std::unique_lock<std::mutex> lock(mutex_endNotificationLoop);
    gcFinished = true;
    cond_endNotification.notify_one();
    lock.unlock();

    //std::cout << "finished and notified... GC " << &gcRunner << std::endl << std::flush;
    xThreadGC.join();
    //std::cout << "deleting GC here." << std::endl << std::flush;
}

void GarbageCollector::loopGC()
{
    std::unique_lock<std::mutex> lock(mutex_endNotificationLoop);
  //  std::cout << "LOOPING gc" << std::endl << std::flush;

    while(!gcFinished)
    {
        if (cond_endNotification.wait_for(lock,Ms(gcPeriodMS)) == std::cv_status::timeout)
        {
           // std::cout << "running gc" << std::endl << std::flush;
            gcRunner(obj);
        }
        else
        {
         //   std::cout << "notified to terminate gc" << std::endl << std::flush;
        }
    }
}

void GarbageCollector::setGCPeriodMS(const uint32_t &msecs)
{
    gcPeriodMS = msecs;
}

