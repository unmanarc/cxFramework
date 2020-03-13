#ifndef MUTEX_RW_H
#define MUTEX_RW_H

#include <pthread.h>

class Mutex_RW
{
public:
    Mutex_RW();
    ~Mutex_RW();

    void lock();
    void unlock();

    void lock_shared();
    void unlock_shared();

private:
    pthread_rwlock_t mutex;

};

#endif // MUTEX_RW_H
