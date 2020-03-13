#ifndef LOCK_MUTEX_RD_H
#define LOCK_MUTEX_RD_H

#include "mutex_rw.h"

class Lock_Mutex_RD
{
public:
    Lock_Mutex_RD(Mutex_RW & mutex, bool dontlock = false);
    ~Lock_Mutex_RD();
private:
    Mutex_RW * mutex;
    bool dontlock;
};

#endif // LOCK_MUTEX_RD_H
