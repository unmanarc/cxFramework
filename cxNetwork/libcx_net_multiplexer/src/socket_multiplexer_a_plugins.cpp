#include "socket_multiplexer.h"

bool Socket_Multiplexer::processMultiplexedSocketCommand_Plugin_JSON16()
{
    bool readen;
    Json::Value jMsg;
    std::string pluginId = multiplexedSocket->readString(&readen,8), sMsg;
    if (!readen) return false;
    sMsg = multiplexedSocket->readString(&readen, 16);
    if (!readen) return false;

    Json::CharReaderBuilder builder;
    Json::CharReader * reader = builder.newCharReader();
    std::string errors;

    if (reader->parse(sMsg.c_str(), sMsg.c_str() + sMsg.size(), &(jMsg), &errors) && plugins.find(pluginId) != plugins.end())
    {
        plugins[pluginId]->processJSON16(jMsg);
    }
    else
    {
        // parse problem?
    }

    delete reader;
    return true;
}

bool Socket_Multiplexer::processMultiplexedSocketCommand_Plugin_Data()
{
    bool readen;
    unsigned int datalen = PLUGIN_MAX_DATA;
    std::string pluginId = multiplexedSocket->readString(&readen,8);
    if (!readen) return false;
    char * pluginData = ((char *)multiplexedSocket->readBlock32WAlloc(&datalen));
    if (!pluginData) return false;
    if (plugins.find(pluginId) != plugins.end())
    {
        plugins[pluginId]->processData(pluginData,datalen);
    }
    delete [] pluginData;
    return true;
}

bool Socket_Multiplexer::plugin_SendData(const std::string &pluginId, void *data, const uint32_t &datalen, bool lock)
{
    if (noSendData) return false;

    if (lock) mtLock_multiplexedSocket.lock();
    if (!multiplexedSocket->writeU8(MPLX_PLUGIN_DATA))
    {
        if (lock) mtLock_multiplexedSocket.unlock();
        return false;
    }
    if (!multiplexedSocket->writeString8(pluginId))
    {
        if (lock) mtLock_multiplexedSocket.unlock();
        return false;
    }
    if (!multiplexedSocket->writeBlock32(data,datalen))
    {
        if (lock) mtLock_multiplexedSocket.unlock();
        return false;
    }
    if (lock) mtLock_multiplexedSocket.unlock();
    return true;
}

bool Socket_Multiplexer::plugin_SendJson(const std::string &pluginId, const Json::Value &jData, bool lock)
{
    if (noSendData) return false;
    if (lock) mtLock_multiplexedSocket.lock();
    if (!multiplexedSocket->writeU8(MPLX_PLUGIN_JSON))
    {
        if (lock) mtLock_multiplexedSocket.unlock();
        return false;
    }
    if (!multiplexedSocket->writeString8(pluginId))
    {
        if (lock) mtLock_multiplexedSocket.unlock();
        return false;
    }
    if (!multiplexedSocket->writeString16(jData.toStyledString()))
    {
        if (lock) mtLock_multiplexedSocket.unlock();
        return false;
    }
    if (lock) mtLock_multiplexedSocket.unlock();
    return true;
}
