#include "lock_mutex_rd.h"

Lock_Mutex_RD::Lock_Mutex_RD(Mutex_RW &mutex, bool dontlock)
{
    this->mutex = &mutex;
    this->dontlock = dontlock;
    if (!dontlock) mutex.lock_shared();
}

Lock_Mutex_RD::~Lock_Mutex_RD()
{
    if (!dontlock) mutex->unlock_shared();
}
