#include "lock_mutex_rw.h"

Lock_Mutex_RW::Lock_Mutex_RW(Mutex_RW &mutex, bool dontlock)
{
    this->mutex = &mutex;
    this->dontlock = dontlock;
    if (dontlock) mutex.lock();
}

Lock_Mutex_RW::~Lock_Mutex_RW()
{
    if (dontlock) mutex->unlock();
}
