#include "socket_stream_bridge.h"

void pipeThread(Socket_Stream_Bridge * stp)
{
    stp->process();
    if (stp->isAutoDeleteStreamPipeOnThreadExit())
    {
        delete stp;
    }
}

Socket_Stream_Bridge::Socket_Stream_Bridge()
{
    bridgeThreadPrc = nullptr;
    sentBytes = 0;
    recvBytes = 0;

    socket_peers[0] = nullptr;
    socket_peers[1] = nullptr;

    finishingPeer = -1;
    autoDeleteSocketsOnExit = false;
    autoDeleteCustomPipeOnClose = false;

    setAutoDeleteStreamPipeOnThreadExit(true);
    setToShutdownRemotePeer(true);
    setToCloseRemotePeer(true);
}

Socket_Stream_Bridge::~Socket_Stream_Bridge()
{
    if (autoDeleteSocketsOnExit)
    {
        if (socket_peers[0]) delete socket_peers[0];
        if (socket_peers[1]) delete socket_peers[1];
    }
}

bool Socket_Stream_Bridge::start(bool _autoDeleteStreamPipeOnExit, bool detach)
{
    if (!socket_peers[0] || !socket_peers[1]) return false;

    autoDeleteStreamPipeOnExit = _autoDeleteStreamPipeOnExit;

    pipeThreadP = std::thread(pipeThread, this);

    if (autoDeleteStreamPipeOnExit || detach)
        pipeThreadP.detach();

    return true;
}

int Socket_Stream_Bridge::wait()
{
    pipeThreadP.join();
    return finishingPeer;
}

void remotePeerThread(Socket_Stream_Bridge * stp)
{
    stp->processPeer(1);
}

int Socket_Stream_Bridge::process()
{
    if (!socket_peers[0] || !socket_peers[1]) return -1;

    if (!bridgeThreadPrc)
    {
        bridgeThreadPrc = new Bridge_Thread();
        autoDeleteCustomPipeOnClose = true;
    }

    bridgeThreadPrc->setStreamSockets(socket_peers[0],socket_peers[1]);

    if (bridgeThreadPrc->startPipeSync())
    {
        std::thread remotePeerThreadP = std::thread(remotePeerThread, this);
        processPeer(0);
        remotePeerThreadP.join();
    }

    // All connections terminated.
    if (closeRemotePeerOnFinish)
    {
        // close them also.
        socket_peers[1]->closeSocket();
        socket_peers[0]->closeSocket();
    }

    if ( autoDeleteCustomPipeOnClose )
    {
        delete bridgeThreadPrc;
        bridgeThreadPrc = nullptr;
    }

    return finishingPeer;
}

bool Socket_Stream_Bridge::processPeer(unsigned char cur)
{
    if (cur>1) return false;

    unsigned char next = cur==0?1:0;
    std::atomic<uint64_t> * bytesCounter = cur==0?&sentBytes:&recvBytes;

    int dataRecv=0;
    while ( dataRecv >= 0 )
    {
        dataRecv = cur==0?bridgeThreadPrc->processPipeFWD():bridgeThreadPrc->processPipeREV();

        if (dataRecv>=0) *bytesCounter+=dataRecv;
        else if (dataRecv==-1 && shutdownRemotePeerOnFinish)
        {
            socket_peers[next]->shutdownSocket();
            finishingPeer = cur;
        }
    }

    return true;
}

bool Socket_Stream_Bridge::setPeer(unsigned char i, Socket_Base_Stream *s)
{
    if (i>1) return false;
    socket_peers[i] = s;
    return true;
}

Socket_Base_Stream *Socket_Stream_Bridge::getPeer(unsigned char i)
{
    if (i>1) return nullptr;
    return socket_peers[i];
}

void Socket_Stream_Bridge::setAutoDeleteStreamPipeOnThreadExit(bool value)
{
    autoDeleteStreamPipeOnExit = value;
}

void Socket_Stream_Bridge::setToShutdownRemotePeer(bool value)
{
    shutdownRemotePeerOnFinish = value;
}

void Socket_Stream_Bridge::setToCloseRemotePeer(bool value)
{
    closeRemotePeerOnFinish = value;
}

uint64_t Socket_Stream_Bridge::getSentBytes() const
{
    return sentBytes;
}

uint64_t Socket_Stream_Bridge::getRecvBytes() const
{
    return recvBytes;
}

bool Socket_Stream_Bridge::isAutoDeleteStreamPipeOnThreadExit() const
{
    return autoDeleteStreamPipeOnExit;
}

bool Socket_Stream_Bridge::isAutoDeleteSocketsOnExit() const
{
    return autoDeleteSocketsOnExit;
}

void Socket_Stream_Bridge::setAutoDeleteSocketsOnExit(bool value)
{
    autoDeleteSocketsOnExit = value;
}


void Socket_Stream_Bridge::setCustomPipeProcessor(Bridge_Thread *value, bool deleteOnExit)
{
    bridgeThreadPrc = value;
}
