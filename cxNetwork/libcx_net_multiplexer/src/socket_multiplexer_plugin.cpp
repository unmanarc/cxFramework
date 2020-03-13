#include "socket_multiplexer_plugin.h"

#include "socket_multiplexer.h"

Socket_Mutiplexer_Plugin::Socket_Mutiplexer_Plugin()
{

}

Socket_Mutiplexer_Plugin::~Socket_Mutiplexer_Plugin()
{

}

std::string Socket_Mutiplexer_Plugin::getPluginId() const
{
    return pluginId;
}

void Socket_Mutiplexer_Plugin::setPluginId(const std::string &value)
{
    pluginId = value;
}

bool Socket_Mutiplexer_Plugin::sendJSON16(const Json::Value &value)
{
    return ((Socket_Multiplexer *)multiplexerPtr)->plugin_SendJson(pluginId, value);
}

void Socket_Mutiplexer_Plugin::setMultiplexerPtr(void *value)
{
    multiplexerPtr = value;
}
