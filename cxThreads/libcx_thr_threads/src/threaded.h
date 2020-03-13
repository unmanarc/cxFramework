#ifndef THREADED_H
#define THREADED_H

#include <thread>
#include <atomic>

class Threaded
{
public:
    Threaded();
    virtual ~Threaded();

    // THREAD ASSOCIATED:
    void start();
    void stop();

    // STATUS:
    bool isRunning() const;

    // CLASS AS POINTER (and deleted when thread finished)
    bool getDestroyOnExit() const;
    void setDestroyOnExit(bool value);

    // CALLEABLE FROM THREAD:
    void execRun();
    void detach();

    void setThreadRunner(void (*thrRunner)(void *objRunner), void *objRunner);
    void setThreadStopper(void (*thrStopper)(void *objRunner), void *objRunner);

private:
    void join();

    void (*thrRunner)(void *objRunner);
    void * objRunner;
    void (*thrStopper)(void *objRunner);
    void * objStopper;

    std::atomic<bool> running, destroyOnExit, detached;
    std::thread xThread;
    int threadRet;
};

#endif // THREADED_H
