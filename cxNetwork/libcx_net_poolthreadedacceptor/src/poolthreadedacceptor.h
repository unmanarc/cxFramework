#ifndef POOLTHREADEDACCEPTOR_H
#define POOLTHREADEDACCEPTOR_H

#include <cx_net_sockets/socket_base_stream.h>
#include <cx_thr_threads/threaded.h>
#include <cx_thr_threadpool/threadpool.h>


// TODO: statistics
/**
 * @brief The PoolThreadedAcceptor class
 */
class PoolThreadedAcceptor : public Threaded
{
public:
    /**
     * @brief PoolThreadedAcceptor Constructor
     * @param acceptorSocket Pre-initialized acceptor socket
     * @param _CallbackTask Callback for succeed inserted task
     * @param _CallbackFailed  Callback when task insertion failed (saturation)
     * @param obj Object to be passed to callbacks
     */
    PoolThreadedAcceptor();
    ~PoolThreadedAcceptor() override;

    /**
     * @brief run Don't call this function, call start(). This is a virtual function for the processor thread.
     */
    void run();
    /**
     * @brief stop Call to stop the acceptor and automatically delete/destroy this class (don't call anything after this).
     */
    void stop();
    /**
     * Set callback when connection is fully established (if the callback returns false, connection socket won't be automatically closed/deleted)
     */
    void setCallbackOnConnect(bool (*_callbackOnConnect)(void *, Socket_Base_Stream *, const char *), void *obj);
    /**
     * Set callback when protocol initialization failed (like bad X.509 on TLS) (if the callback returns false, connection socket won't be automatically closed/deleted)
     */
    void setCallbackOnInitFail(bool (*_callbackOnInitFailed)(void *, Socket_Base_Stream *, const char *), void *obj);
    /**
     * Set callback when timed out (all the thread queues are saturated) (this callback is called from acceptor thread, you should use it very quick)
     */
    void setCallbackOnTimedOut(void (*_callbackOnTimedOut)(void *, Socket_Base_Stream *, const char *), void *obj);


    /////////////////////////////////////////////////////////////////////////
    // TUNNING:

    uint32_t getTimeoutMS() const;
    /**
     * @brief setTimeoutMS Set the timeout in milliseconds
     * @param value timeout to cease to try to insert the task in a queue
     */
    void setTimeoutMS(const uint32_t &value);

    ////////

    uint32_t getThreadsCount() const;
    /**
     * @brief setThreadsCount Set how many threads will be used (call before start)
     * @param value thread count
     */
    void setThreadsCount(const uint32_t &value);

    ////////

    uint32_t getTaskQueues() const;
    /**
     * @brief setTaskQueues Set how many queues to store tasks, each queue handle 100 tasks in wait mode.
     * @param value queues count
     */
    void setTaskQueues(const uint32_t &value);

    ////////

    float getQueuesKeyRatio() const;
    /**
     * @brief setQueuesKeyRatio Set how many queues can be used by some key (in this case, key is the source ip address)
     *                          using all queues means that there is no mechanism to prevent saturation.
     * @param value number from (0-1], 0.0 means 1 queue, and 1 is for all queues, default value: 0.5 (half)
     */
    void setQueuesKeyRatio(float value);

    void setAcceptorSocket(Socket_Base_Stream *value);


private:
    ThreadPool * pool;
    Socket_Base_Stream * acceptorSocket;

    bool (*callbackOnConnect)(void *,Socket_Base_Stream *, const char *);
    bool (*callbackOnInitFail)(void *,Socket_Base_Stream *, const char *);
    void (*callbackOnTimedOut)(void *,Socket_Base_Stream *, const char *);

    void *objOnConnect, *objOnInitFail, *objOnTimedOut;

    float queuesKeyRatio;
    uint32_t timeoutMS;
    uint32_t threadsCount;
    uint32_t taskQueues;
};

#endif // POOLTHREADEDACCEPTOR_H
