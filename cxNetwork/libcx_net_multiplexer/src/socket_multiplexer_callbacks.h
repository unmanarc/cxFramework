#ifndef SOCKET_MULTIPLEXER_CALLBACKS_H
#define SOCKET_MULTIPLEXER_CALLBACKS_H

#include <stdint.h>
#include <memory>
#include <json/json.h>
#include "socket_multiplexed_line.h"

enum eConnectFailedReason {
    E_CONN_FAILED=0x00,
    E_CONN_FAILED_TIMEOUT=0x01,
    E_CONN_FAILED_ANSTHREAD=0x02,
    E_CONN_FAILED_BADPARAMS=0x03,
    E_CONN_FAILED_NOCALLBACK = 0x5,
    E_CONN_FAILED_BADSERVERSOCK = 0x6,
    E_CONN_FAILED_BADLOCALLINE = 0x7,
    E_CONN_FAILED_NOTAUTHORIZED = 0x8,
    E_CONN_OK=0xFF
};

struct sServerConnectAcceptCallback
{
    sServerConnectAcceptCallback()
    {
        callbackFunction=nullptr;
        obj = nullptr;
    }
    Socket_Base_Stream * (*callbackFunction)(void *, const LineID &, const Json::Value &);
    void *obj;
};

struct sClientConnectAcceptedCallback
{
    sClientConnectAcceptedCallback()
    {
        callbackFunction=nullptr;
        obj = nullptr;
    }
    Socket_Base_Stream * (*callbackFunction)(void *, std::shared_ptr<Socket_Multiplexed_Line>);
    void *obj;
};

struct sClientConnectFailedCallback
{
    sClientConnectFailedCallback()
    {
        callbackFunction=nullptr;
        obj = nullptr;
    }
    // obj, socket, failed reason
    bool (*callbackFunction)(void *, std::shared_ptr<Socket_Multiplexed_Line>, eConnectFailedReason);
    void *obj;
};

struct sServerConnectionFinishedCallback
{
    sServerConnectionFinishedCallback()
    {
        callbackFunction=nullptr;
        obj = nullptr;
    }
    // obj, socket, failed reason
    void (*callbackFunction)(void *, const LineID &, Socket_Base_Stream *);
    void *obj;
};

class Socket_Multiplexer_Callbacks
{
public:
    Socket_Multiplexer_Callbacks();
    //////////////////////////////////
    // callback systems:
    void setCallback_ServerConnectAcceptor(Socket_Base_Stream * (*callbackFunction)(void *, const LineID &, const Json::Value &), void *obj=nullptr);
    void setCallback_ServerConnectionFinished(void (*callbackFunction)(void *, const LineID &, Socket_Base_Stream *), void *obj=nullptr);
    void setCallback_ClientConnectAccepted(Socket_Base_Stream * (*callbackFunction)(void *, std::shared_ptr<Socket_Multiplexed_Line>), void * obj=nullptr);
    void setCallback_ClientConnectFailed(bool (*callbackFunction)(void *, std::shared_ptr<Socket_Multiplexed_Line>, eConnectFailedReason), void * obj=nullptr);

protected:

    // callbacks:
    sServerConnectAcceptCallback    cbServerConnectAcceptor;
    sClientConnectAcceptedCallback  cbClientConnectAccepted;
    sClientConnectFailedCallback    cbClientConnectFailed;
    sServerConnectionFinishedCallback cbServerConnectionFinished;

};

#endif // SOCKET_MULTIPLEXER_CALLBACKS_H
