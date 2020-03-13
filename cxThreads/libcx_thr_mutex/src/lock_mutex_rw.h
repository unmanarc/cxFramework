#ifndef LOCK_MUTEX_RW_H
#define LOCK_MUTEX_RW_H

#include "mutex_rw.h"

class Lock_Mutex_RW
{
public:
    Lock_Mutex_RW(Mutex_RW & mutex, bool dontlock = false);
    ~Lock_Mutex_RW();
private:
    Mutex_RW * mutex;
    bool dontlock;
};

#endif // LOCK_MUTEX_RW_H
