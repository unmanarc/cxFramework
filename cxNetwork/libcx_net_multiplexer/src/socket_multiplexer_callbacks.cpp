#include "socket_multiplexer_callbacks.h"

Socket_Multiplexer_Callbacks::Socket_Multiplexer_Callbacks()
{

}

void Socket_Multiplexer_Callbacks::setCallback_ClientConnectAccepted(Socket_Base_Stream * (*callbackFunction)(void *, std::shared_ptr<Socket_Multiplexed_Line>), void *obj)
{
    cbClientConnectAccepted.callbackFunction = callbackFunction;
    cbClientConnectAccepted.obj = obj;
}

void Socket_Multiplexer_Callbacks::setCallback_ClientConnectFailed(bool (*callbackFunction)(void *, std::shared_ptr<Socket_Multiplexed_Line>, eConnectFailedReason), void *obj)
{
    cbClientConnectFailed.callbackFunction = callbackFunction;
    cbClientConnectFailed.obj = obj;
}

void Socket_Multiplexer_Callbacks::setCallback_ServerConnectAcceptor(Socket_Base_Stream * (*callbackFunction)(void *, const LineID &, const Json::Value &), void *obj)
{
    cbServerConnectAcceptor.obj = obj;
    cbServerConnectAcceptor.callbackFunction = callbackFunction;
}

void Socket_Multiplexer_Callbacks::setCallback_ServerConnectionFinished(void (*callbackFunction)(void *, const LineID &, Socket_Base_Stream *), void *obj)
{
    cbServerConnectionFinished.obj = obj;
    cbServerConnectionFinished.callbackFunction = callbackFunction;
}
