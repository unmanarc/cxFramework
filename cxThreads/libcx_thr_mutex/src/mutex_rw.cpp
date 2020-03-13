#include "mutex_rw.h"

Mutex_RW::Mutex_RW()
{
    mutex = PTHREAD_RWLOCK_INITIALIZER;
}

Mutex_RW::~Mutex_RW()
{
    pthread_rwlock_destroy(&mutex);
}

void Mutex_RW::lock()
{
    pthread_rwlock_wrlock(&mutex);
}

void Mutex_RW::unlock()
{
    pthread_rwlock_unlock(&mutex);
}

void Mutex_RW::lock_shared()
{
    pthread_rwlock_rdlock(&mutex);
}

void Mutex_RW::unlock_shared()
{
    unlock();
}
