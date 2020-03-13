#ifndef MUTEX_H
#define MUTEX_H

#include <mutex>

typedef std::mutex Mutex;
#define Lock_Mutex(x) std::lock_guard(x)

#endif // MUTEX_H
