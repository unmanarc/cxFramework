#include "socketchainbase.h"

SocketChainBase::SocketChainBase()
{
    serverMode = false;
}

SocketChainBase::~SocketChainBase()
{

}

bool SocketChainBase::isEndPoint()
{
    return false;
}

std::pair<Socket_Base_Stream *, Socket_Base_Stream *> SocketChainBase::makeSocketChainPair()
{
    std::pair<Socket_Base_Stream *, Socket_Base_Stream *> pair = Socket_Base_Stream::GetSocketPair();
    if (pair.first == nullptr) return pair;

    // Transfer the socket from pairsocket to the chained implementation...
    Socket_Base_Stream * realSock = (Socket_Base_Stream *)getThis();
    realSock->setSocket(pair.first->getSocket());
    pair.first->detachSocket();
    delete pair.first;
    pair.first = realSock;

    return pair;
}

bool SocketChainBase::isServerMode() const
{
    return serverMode;
}

void SocketChainBase::setServerMode(bool value)
{
    serverMode = value;
}

