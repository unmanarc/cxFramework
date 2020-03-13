#include "socket_multiplexer.h"

Socket_Multiplexer::Socket_Multiplexer()
{
    mtLock_multiplexedSocket.lock();

    memset(readData,0,65536);

    localName = "localhost";
    noSendData = true;

    destroySocketOnClient=false;
    destroySocketOnServer=false;

    peerMultiplexorVersion = 0;
    multiplexedSocket=nullptr;
}

Socket_Multiplexer::~Socket_Multiplexer()
{
    mtLock_multiplexedSocket.unlock();
    for (auto i : plugins)
    {
        delete i.second;
    }
}

bool Socket_Multiplexer::plugin_Add(Socket_Mutiplexer_Plugin *plugin)
{
    if (plugins.find(plugin->getPluginId())==plugins.end())
    {
        plugin->setMultiplexerPtr(this);
        plugins[plugin->getPluginId()] = plugin;
        return true;
    }
    return false;
}

void Socket_Multiplexer::start(Socket_Base_Stream *multiplexedSocket, const std::string & localName)
{  
    bool readOK;
    this->multiplexedSocket = multiplexedSocket;
    this->localName = localName;

    unsigned char chIdSize = sizeof(LineID);

    if (multiplexedSocket->writeU16(SOCKET_MULTIPLEXER_VERSION) && multiplexedSocket->writeU8(chIdSize))
    {
        peerMultiplexorVersion = multiplexedSocket->readU16(&readOK);
        if (readOK)
        {
            unsigned char channelSize = multiplexedSocket->readU8(&readOK);
            if (readOK && channelSize == chIdSize)
            {
                if (multiplexedSocket->writeString8(localName))
                {
                    this->remoteName = multiplexedSocket->readString(&readOK,8);
                    if (readOK)
                    {
                        for (auto & i : plugins)
                            ((Socket_Mutiplexer_Plugin *)i.second)->eventOnMultiplexedSocketConnect();
                        unlockNewConnections();
                        noSendData = false;
                        mtLock_multiplexedSocket.unlock();
                        while (processMultiplexedSocket()) {}
                        noSendData = true;
                        mtLock_multiplexedSocket.lock();
                    }
                }
            }
        }
    }

    // TODO: check this block...
    mtLock_multiplexedSocket.unlock();
    // Connections still trying to send something? sending nothing from here.
    for (auto & i : plugins)
        ((Socket_Mutiplexer_Plugin *)i.second)->eventOnMultiplexedSocketFinalization();
    //forceCloseAllChannels();
    preventNewConnections();
    closeAndWaitForEveryLine();
    // Here there are no new and no connections (not from connect by lockNewConnections, and not by accept be processMultiplexedSocket)...
    // So, this lock will be the last...
    mtLock_multiplexedSocket.lock();

    multiplexedSocket->shutdownSocket();
    multiplexedSocket->closeSocket();
    multiplexedSocket = nullptr;
}

bool Socket_Multiplexer::forceClose()
{
    multiplexedSocket->shutdownSocket();
    return true;
}

bool Socket_Multiplexer::processMultiplexedSocket()
{
    bool readen;
    eMultiplexedSocketMessage msg = (eMultiplexedSocketMessage)multiplexedSocket->readU8(&readen);
    if (!readen) return false;

    switch(msg)
    {
    case MPLX_CLOSE_ACK2:
    {
//        multiplexedSocket->shutdownSocket();
    } return false;
    case MPLX_CLOSE_ACK1:
    {
        // Ordered back multiplexed socket close (reverse).
        // all the connections in the remote side are closed now.
        // closing this side (if there is one)
        //closeAndWaitForEveryLine();
        if (!multiplexedSocket_sendCloseACK2()) return false;
        //multiplexedSocket->shutdownSocket();
    } return false;
    case MPLX_MSG_CLOSE:
    {
        // Ordered multiplexed socket Close.
        //closeAndWaitForEveryLine();
        if (!multiplexedSocket_sendCloseACK1())
            return false;
    } break;
    case MPLX_LINE_CONNECT:
    {
        // line connection start
        bool authorized = true;
        for (auto & i : plugins)
        {
            if (!((Socket_Mutiplexer_Plugin *)i.second)->eventOnLineConnect())
            {
                authorized = false;
                break;
            }
        }
        if (!processMultiplexedSocketCommand_Line_Connect(authorized))
            return false;
    } break;
    case MPLX_PLUGIN_DATA:
    {
        // multiplexed socket recv plugin data
        if (!processMultiplexedSocketCommand_Plugin_Data())
            return false;
    } break;
    case MPLX_PLUGIN_JSON:
    {
        // multiplexed socket recv plugin json (up to 64kb)
        if (!processMultiplexedSocketCommand_Plugin_JSON16())
            return false;
    } break;
    case MPLX_LINE_CONNECT_ANS:
    {
        // multiplexed socket recv connection answer
        if (!processMultiplexedSocketCommand_Line_ConnectionAnswer())
            return false;
    } break;
    case MPLX_LINE_DATA:
    {
        // multiplexed socket recv line data.
        if (!processMultiplexedSocketCommand_Line_Data())
            return false;
    } break;
    case MPLX_LINE_BYTESREADEN:
    {
        // multiplexed socket update bytes readen
        if (!processMultiplexedSocketCommand_Line_UpdateReadenBytes())
            return false;
    } break;
    default:
        break;
    }

    return true;
}

bool Socket_Multiplexer::sendOnMultiplexedSocket_LineID(const LineID &chId)
{
    if (sizeof(LineID) == 1)
        return multiplexedSocket->writeU8(chId);
    else if (sizeof(LineID) == 2)
        return multiplexedSocket->writeU16(chId);
    else
        return multiplexedSocket->writeU32(chId);
}

LineID Socket_Multiplexer::recvFromMultiplexedSocket_LineID(bool * readen)
{
    if (sizeof(LineID) == 1)
        return multiplexedSocket->readU8(readen);
    else if (sizeof(LineID) == 2)
        return multiplexedSocket->readU16(readen);
    else
        return multiplexedSocket->readU32(readen);
}

bool Socket_Multiplexer::close()
{
    // send into multiplexed socket the close message, peer must close the connection.
    std::unique_lock<std::timed_mutex> lock(mtLock_multiplexedSocket);
    return multiplexedSocket->writeU8(MPLX_MSG_CLOSE);
}


bool Socket_Multiplexer::multiplexedSocket_sendCloseACK1()
{
    // send into multiplexed socket the close message acknowledge, peer must close the connection.
    std::unique_lock<std::timed_mutex> lock(mtLock_multiplexedSocket);
    return multiplexedSocket->writeU8(MPLX_CLOSE_ACK1);
}

bool Socket_Multiplexer::multiplexedSocket_sendCloseACK2()
{
    // send into multiplexed socket the close message, host must close the connection.
    std::unique_lock<std::timed_mutex> lock(mtLock_multiplexedSocket);
    return multiplexedSocket->writeU8(MPLX_CLOSE_ACK2);
}

bool Socket_Multiplexer::getDestroySocketOnServer() const
{
    return destroySocketOnServer;
}

void Socket_Multiplexer::setDestroySocketOnServer(bool value)
{
    destroySocketOnServer = value;
}

bool Socket_Multiplexer::getDestroySocketOnClient() const
{
    return destroySocketOnClient;
}

void Socket_Multiplexer::setDestroySocketOnClient(bool value)
{
    destroySocketOnClient = value;
}

std::string Socket_Multiplexer::getLocalName() const
{
    return localName;
}

std::string Socket_Multiplexer::getRemoteName() const
{
    return remoteName;
}
