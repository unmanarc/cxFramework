#include "jasyncrpc_server.h"
#include <cx_jasyncrpc_common/jasyncrpc_retcodes.h>

JAsyncRPC_Server::JAsyncRPC_Server(StreamableObject *sobject) : StreamParser(sobject,false)
{
    // Set current parser for .parse() function
    currentParser = (SubStreamParser *)(&remoteHandshake);

    bIsValidHandShake = false;

    // Create a thread for dequeing replies
    request.initElemParser(sobject,false);
    localHandshake.initElemParser(sobject,false);
    remoteHandshake.initElemParser(sobject,false);
}

JAsyncRPC_Server::~JAsyncRPC_Server()
{
}

void JAsyncRPC_Server::setAuthenticator(IAuth *authenticator)
{
    // This server is one per client, create a session here.
    authSession.setAuthenticator(authenticator);
}

IAuth_Session *JAsyncRPC_Server::getAuthSession()
{
    return &authSession;
}

bool JAsyncRPC_Server::sendAnswer(JAsyncRPC_Request & response)
{
    std::unique_lock<std::mutex> lock(mutex_send);
    WRStatus wrStat;
    response.initElemParser(streamableObject,false);
    return response.stream(wrStat);
}

bool JAsyncRPC_Server::initProtocol()
{
    std::unique_lock<std::mutex> lock(mutex_send);
    WRStatus wrStat;
    // server send handshake...;
    return localHandshake.stream(wrStat);
}

bool JAsyncRPC_Server::changeToNextParser()
{
    if (currentParser == &remoteHandshake)
    {
        // Validate handshake...
        if (validateHandshake())
        {
            bIsValidHandShake = true;
            currentParser = &request;
        }
        else
        {
            currentParser = nullptr; // END
            bIsValidHandShake = false;
        }
    }
    else if (currentParser == &request)
    {
        // process request...
        if (processRequest())
            request.clear();
        else
            currentParser = nullptr; // END
    }
    return true;
}

bool JAsyncRPC_Server::validateHandshake()
{
    if (remoteHandshake.getProtocolVersion()!=localHandshake.getProtocolVersion())
        return false;
    return true;
}

bool JAsyncRPC_Server::processRequest()
{
/*    std::cout << "Receiving from client: -------------------------------------" << std::endl << std::flush;
    request.print();
    std::cout << "------------------------------------------------------------" << std::endl << std::flush;*/
    if (request.getRpcMode() == "QUIT")
        return false;
    else if (request.getRpcMode() == "AUTH")
    {
        // Login for the first time...
        if (request.getMethodName() == "LOGIN")
        {
            JAsyncRPC_Authentication auth = request.getAuthentication(0);
            persistentAuthentication(auth);
        }
    }
    else if (request.getRpcMode() == "EXEC")
    {
        JAsyncRPC_Request answer;
        answer.setReqId(request.getReqId());
        answer.setMethodName(request.getMethodName());
        answer.setRpcMode("EXEC");

        // First: take authentications...
        std::set<uint32_t> extraTmpIndexes;
        for (const uint32_t & passIdx : request.getAuthenticationsIdxs())
        {
            if (temporaryAuthentication(request.getAuthentication(passIdx)))
            {
                extraTmpIndexes.insert(passIdx);
            }
            else
                return true; // bad auth.
        }

        Json::Value reasons;
        switch (methodsManager->validateRPCMethodPerms( &authSession, request.getMethodName(), extraTmpIndexes, &reasons))
        {
        case VALIDATION_OK:
        {
        }break;
        case VALIDATION_NOTAUTHORIZED:
        {
            // not enough permissions.
            answer.setExtraInfo(reasons);
            answer.setRetCode(METHOD_RET_CODE_INVALIDAUTH);
            return sendAnswer( answer );
        }
        case VALIDATION_METHODNOTFOUND:
        {
            // not enough permissions.
            answer.setRetCode(METHOD_RET_CODE_METHODNOTFOUND);
            return sendAnswer( answer );
        }
        }
        sRPCParameters * rpcParameters = new sRPCParameters;
        if (rpcParameters)
        {
            rpcParameters->auth = &authSession;
            rpcParameters->extraInfo = request.getExtraInfo();
            rpcParameters->methodName = request.getMethodName();
            rpcParameters->payload = request.getPayload();
            rpcParameters->connectionSender = this;
            rpcParameters->rpcMethodsCaller = methodsManager;
            rpcParameters->requestId = request.getReqId();

            //std::cout << "introduciendo " << rpcParameters << std::endl << std::flush;

            if (!methodsManager->pushRPCMethodIntoQueue(rpcParameters, authSession.getAuthUser()))
            {
                answer.setRetCode(METHOD_RET_CODE_TIMEDOUT);
                delete rpcParameters;
                return sendAnswer( answer );
            }
        }
        else
        {
            // Memory full
            answer.setRetCode(METHOD_RET_CODE_SERVERMEMORYFULL);
            return sendAnswer( answer );
        }
    }
    return true;
}

bool JAsyncRPC_Server::temporaryAuthentication(const JAsyncRPC_Authentication &authData)
{
    AuthReason reason;

    Json::Value payload, extraInfo;
    reason = authSession.getAuthenticator()->authenticate( authSession.getAuthUser(),"",authData.getUserPass(),authData.getPassIndex()); // Authenticate in a non-persistent way.

    if ( reason != AUTH_REASON_AUTHENTICATED )
    {
        extraInfo["reasonTxt"] = getAuthReasonText(reason);
        extraInfo["reasonVal"] = reason;
        extraInfo["passIndex"] = authData.getPassIndex();

        JAsyncRPC_Request response;
        response.setRpcMode("EXEC");
        response.setMethodName(request.getMethodName());
        response.setReqId(request.getReqId());
        response.setExtraInfo(extraInfo);
        response.setRetCode(METHOD_RET_CODE_INVALIDLOCALAUTH);

        sendAnswer(response);

        return false;
    }
    return true;
}

void JAsyncRPC_Server::persistentAuthentication(const JAsyncRPC_Authentication &authData)
{
    Json::Value payload;
    AuthReason authReason = authSession.authenticate(authData.getUserName(),"",authData.getUserPass(),authData.getPassIndex());

    if (authReason==AUTH_REASON_AUTHENTICATED)
        setUserId(authData.getUserName());

    payload["reasonTxt"] = getAuthReasonText(authReason);
    payload["reasonVal"] = static_cast<Json::UInt>(authReason);
    payload["passIndex"] = authData.getPassIndex();

    JAsyncRPC_Request response;
    response.setRpcMode("AUTH");
    response.setMethodName("LOGIN");
    response.setPayload(payload);

    sendAnswer(response);
}

void JAsyncRPC_Server::setMethodsManager(JAsyncRPC_Server_Methods *value)
{
    methodsManager = value;
}

void JAsyncRPC_Server::setUserId(const std::string &value)
{
    authSession.setAuthUser(value);
}

bool JAsyncRPC_Server::isValidHandShake() const
{
    return bIsValidHandShake;
}

JAsyncRPC_Handshake * JAsyncRPC_Server::getRemoteHandshake()
{
    return &remoteHandshake;
}

JAsyncRPC_Handshake * JAsyncRPC_Server::getLocalHandshake()
{
    return &localHandshake;
}
