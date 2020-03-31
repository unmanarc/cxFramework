#include "poolthreadedacceptor.h"
#include <string.h>

struct sAcceptorTaskData
{
    ~sAcceptorTaskData()
    {
        memset(remotePair,0,INET6_ADDRSTRLEN+1);
        if (clientSocket)
        {
            clientSocket->shutdownSocket();
            clientSocket->closeSocket();
            delete clientSocket;
            clientSocket = nullptr;
        }
    }

    bool (*callbackOnConnect)(void *,Socket_Base_Stream *, const char *);
    bool (*callbackOnInitFail)(void *,Socket_Base_Stream *, const char *);
    void *objOnConnect, *objOnInitFail;

    std::string key;

    void * obj;
    Socket_Base_Stream * clientSocket;
    char remotePair[INET6_ADDRSTRLEN+1];
    char pad;
};

void acceptorTask(void * data)
{
    sAcceptorTaskData * taskData = ((sAcceptorTaskData *)data);
    if (taskData->clientSocket->postAcceptSubInitialization())
    {
        // Start
        if (taskData->callbackOnConnect)
        {
            if (!taskData->callbackOnConnect(taskData->objOnConnect, taskData->clientSocket, taskData->remotePair))
            {
                taskData->clientSocket = nullptr;
            }
        }
    }
    else
    {
        if (taskData->callbackOnInitFail)
        {
            if (!taskData->callbackOnInitFail(taskData->objOnInitFail, taskData->clientSocket, taskData->remotePair))
            {
                taskData->clientSocket = nullptr;
            }
        }
    }
    delete taskData;
}


void runner(void * data)
{
    ((PoolThreadedAcceptor *)data)->run();
}

void stopper(void * data)
{
    ((PoolThreadedAcceptor *)data)->stop();
}

PoolThreadedAcceptor::PoolThreadedAcceptor()
{
    this->pool = nullptr;
    this->acceptorSocket = nullptr;


    callbackOnConnect = nullptr;
    callbackOnInitFail = nullptr;
    callbackOnTimedOut = nullptr;

    objOnConnect = nullptr;
    objOnInitFail = nullptr;
    objOnTimedOut = nullptr;

    setThreadRunner(runner,this);
    setThreadStopper(stopper,this);

    setThreadsCount(52);
    setTaskQueues(36);
    setTimeoutMS(5000);
    setQueuesKeyRatio(0.5);
}

void PoolThreadedAcceptor::setCallbackOnConnect(bool (*_callbackOnConnect)(void *, Socket_Base_Stream *, const char *), void *obj)
{
    this->callbackOnConnect = _callbackOnConnect;
    this->objOnConnect = obj;
}

void PoolThreadedAcceptor::setCallbackOnInitFail(bool (*_callbackOnInitFailed)(void *, Socket_Base_Stream *, const char *), void *obj)
{
    this->callbackOnInitFail = _callbackOnInitFailed;
    this->objOnInitFail = obj;
}

void PoolThreadedAcceptor::setCallbackOnTimedOut(void (*_callbackOnTimedOut)(void *, Socket_Base_Stream *, const char *), void *obj)
{
    this->callbackOnTimedOut = _callbackOnTimedOut;
    this->objOnTimedOut = obj;
}


PoolThreadedAcceptor::~PoolThreadedAcceptor()
{
    if (this->pool)
        delete this->pool;
    else
        delete acceptorSocket;
}

void PoolThreadedAcceptor::run()
{
    this->pool = new ThreadPool(threadsCount,taskQueues);
    pool->start();
    for(;;)
    {
        Socket_Base_Stream * clientSocket = acceptorSocket->acceptConnection();
        if (clientSocket)
        {
            sAcceptorTaskData * taskData = new sAcceptorTaskData;
            clientSocket->getRemotePair(taskData->remotePair);
            taskData->callbackOnConnect = callbackOnConnect;
            taskData->callbackOnInitFail = callbackOnInitFail;
            taskData->objOnConnect = objOnConnect;
            taskData->objOnInitFail = objOnInitFail;
            taskData->clientSocket = clientSocket;

            taskData->key = taskData->remotePair;

            if (!pool->pushTask( &acceptorTask, taskData, timeoutMS, queuesKeyRatio, taskData->key))
            {
                if (callbackOnTimedOut!=nullptr) callbackOnTimedOut(objOnTimedOut,clientSocket,taskData->remotePair);
                delete taskData;
            }
        }
        else
            break;
    }
    delete acceptorSocket;
}

void PoolThreadedAcceptor::stop()
{
    acceptorSocket->shutdownSocket();
}

uint32_t PoolThreadedAcceptor::getTimeoutMS() const
{
    return timeoutMS;
}

void PoolThreadedAcceptor::setTimeoutMS(const uint32_t &value)
{
    timeoutMS = value;
}

uint32_t PoolThreadedAcceptor::getThreadsCount() const
{
    return threadsCount;
}

void PoolThreadedAcceptor::setThreadsCount(const uint32_t &value)
{
    threadsCount = value;
}

uint32_t PoolThreadedAcceptor::getTaskQueues() const
{
    return taskQueues;
}

void PoolThreadedAcceptor::setTaskQueues(const uint32_t &value)
{
    taskQueues = value;
}

float PoolThreadedAcceptor::getQueuesKeyRatio() const
{
    return queuesKeyRatio;
}

void PoolThreadedAcceptor::setQueuesKeyRatio(float value)
{
    queuesKeyRatio = value;
}

void PoolThreadedAcceptor::setAcceptorSocket(Socket_Base_Stream *value)
{
    acceptorSocket = value;
}
