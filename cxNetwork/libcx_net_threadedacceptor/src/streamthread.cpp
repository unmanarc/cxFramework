#include "streamthread.h"
#include "threadedstreamacceptor.h"

#include <netinet/in.h>
#include <arpa/inet.h>

#include <string.h>

void thread_streamclient(StreamThread * threadClient, ThreadedStreamAcceptor * threadedAcceptedControl)
{
    threadClient->postInitConnection();
    threadedAcceptedControl->finalizeThreadElement(threadClient);
}

StreamThread::StreamThread()
{
    memset(remotePair,0,INET6_ADDRSTRLEN+2);

    callbackOnConnect = nullptr;
    callbackOnInitFail = nullptr;

    objOnConnect = nullptr;
    objOnInitFail = nullptr;
    parent = nullptr;
    clientSocket = nullptr;
}

StreamThread::~StreamThread()
{
    if (clientSocket)
    {
        clientSocket->closeSocket(); // close the socket when the thread ends...
        delete clientSocket;
        clientSocket = nullptr;
    }
}

void StreamThread::start()
{
    std::thread(thread_streamclient,this,(ThreadedStreamAcceptor *)parent).detach();
}

void StreamThread::stopSocket()
{
    clientSocket->shutdownSocket();
}

void StreamThread::setCallbackOnConnect(bool(*_callbackOnConnect)(void *, Socket_Base_Stream *, const char *), void *obj)
{
    this->callbackOnConnect = _callbackOnConnect;
    this->objOnConnect = obj;
}

void StreamThread::setCallbackOnInitFail(bool (*_callbackOnInitFailed)(void *, Socket_Base_Stream *, const char *), void *obj)
{
    this->callbackOnInitFail = _callbackOnInitFailed;
    this->objOnInitFail = obj;
}


void StreamThread::setParent(void *parent)
{
    this->parent = parent;
}

void StreamThread::postInitConnection()
{
    // Accept (internal protocol)
    if (clientSocket->postAcceptSubInitialization())
    {
        // Start
        if (callbackOnConnect)
        {
            if (!this->callbackOnConnect(objOnConnect, clientSocket, remotePair))
            {
                clientSocket = nullptr;
            }
        }
    }
    else
    {
        if (callbackOnInitFail)
        {
            if (!this->callbackOnInitFail(objOnInitFail, clientSocket, remotePair))
            {
                clientSocket = nullptr;
            }
        }
    }
}

void StreamThread::setSocket(Socket_Base_Stream *_clientSocket)
{
    clientSocket = _clientSocket;
    clientSocket->getRemotePair(remotePair);
}

const char *StreamThread::getRemotePair()
{
    return remotePair;
}
