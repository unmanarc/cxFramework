#ifndef SOCKET_MULTIPLEXER_A_STRUCT_THREADPARAMS_H
#define SOCKET_MULTIPLEXER_A_STRUCT_THREADPARAMS_H

#include <json/json.h>
#include <memory>
#include <stdint.h>

#include "socket_multiplexer_callbacks.h"

struct sServerLineInitThreadParams {
    sServerLineInitThreadParams()
    {
        multiPlexer = nullptr;
    }
    sLineID lineID;
    uint32_t remoteWindowSize;
    Json::Value jConnectionParams;
    void * multiPlexer;
};

struct sConnectionThreadParams {
    sConnectionThreadParams()
    {
        multiPlexer = nullptr;
        reason = E_CONN_OK;
    }
    std::shared_ptr<Socket_Multiplexed_Line> chSock;
    eConnectFailedReason reason;
    void * multiPlexer;
};


#endif // SOCKET_MULTIPLEXER_A_STRUCT_THREADPARAMS_H
