#ifndef JASYNCRPC_SERVER_H
#define JASYNCRPC_SERVER_H

#include "jasyncrpc_server_methods.h"

#include <cx_jasyncrpc_common/jasyncrpc_handshake.h>
#include <cx_jasyncrpc_common/jasyncrpc_request.h>

#include <cx_auth/iauth.h>
#include <cx_auth/iauth_session.h>

#include <cx_mem_streamparser/streamparser.h>

#include <mutex>

class JAsyncRPC_Server : public StreamParser
{
public:
    JAsyncRPC_Server(StreamableObject *sobject);
    ~JAsyncRPC_Server() override;

    //////////////////////////////////////////////
    // Initialization:
    void setAuthenticator(IAuth * authenticator);
    void setMethodsManager(JAsyncRPC_Server_Methods *value);
    //////////////////////////////////////////////

    IAuth_Session * getAuthSession();

    // TODO:
    /*
    bool createChannel(const std::string & channelName); // TODO: how to create auth?
    void broadcastToChannel(const std::string & channelName, const Json::Value & payload);*/

    bool sendAnswer(JAsyncRPC_Request &response);

    JAsyncRPC_Handshake *getLocalHandshake();
    JAsyncRPC_Handshake *getRemoteHandshake();

    bool isValidHandShake() const;

protected:
    bool initProtocol() override;
    void endProtocol() override {}
    void * getThis() { return this; }
    bool changeToNextParser() override;

private:
    void persistentAuthentication(const JAsyncRPC_Authentication &authData);
    bool temporaryAuthentication(const JAsyncRPC_Authentication &authData);

    bool validateHandshake();
    bool processRequest();
    void setUserId(const std::string &value);

    JAsyncRPC_Request request;
    JAsyncRPC_Handshake localHandshake, remoteHandshake;

    JAsyncRPC_Server_Methods * methodsManager;
    IAuth_Session authSession;
    bool bIsValidHandShake;
    std::mutex mutex_send;
};

#endif // JASYNCRPC_SERVER_H
