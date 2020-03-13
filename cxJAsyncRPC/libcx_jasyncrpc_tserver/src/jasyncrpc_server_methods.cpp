#include "jasyncrpc_server_methods.h"

#include <cx_jasyncrpc_common/jasyncrpc_request.h>
#include <cx_jasyncrpc_common/jasyncrpc_retcodes.h>

#include "jasyncrpc_server.h"

void executeRPC(void * taskData)
{
    sRPCParameters * data = (sRPCParameters *)(taskData);


    JAsyncRPC_Request answer;
    answer.setReqId(data->requestId);
    answer.setMethodName(data->methodName);
    answer.setRpcMode("EXEC");

    Json::Value rPayload, rExtraInfo;
    int rcode = ((JAsyncRPC_Server_Methods *)(data->rpcMethodsCaller))->runRPCMethod(data->auth,
                                                                                              data->methodName,
                                                                                              data->payload,
                                                                                              data->extraInfo,
                                                                                              &rPayload,
                                                                                              &rExtraInfo);
    answer.setPayload(rPayload);
    answer.setExtraInfo(rExtraInfo);
    answer.setRetCode(rcode);

    ((JAsyncRPC_Server *)(data->connectionSender))->sendAnswer(answer);
    delete data;
}

JAsyncRPC_Server_Methods::JAsyncRPC_Server_Methods(uint32_t threadsCount, uint32_t taskQueues)
{
    threadPool = new ThreadPool(threadsCount, taskQueues);
    timeout = 2000; // 2sec.
    threadPool->start();
}

JAsyncRPC_Server_Methods::~JAsyncRPC_Server_Methods()
{
    delete threadPool;
}

void JAsyncRPC_Server_Methods::stop()
{
    threadPool->stop();
}

bool JAsyncRPC_Server_Methods::pushRPCMethodIntoQueue(sRPCParameters *params, const std::string &key, const float &priority)
{
    params->rpcMethodsCaller = this;
    return threadPool->pushTask(executeRPC,params,timeout,priority,key);
}

bool JAsyncRPC_Server_Methods::addRPCMethod(const std::string & methodName, const std::list<std::string> &reqAttribs, Json::Value (*rpcMethod)(void *, IAuth_Session *, const Json::Value &,const Json::Value &,Json::Value *), void * obj)
{
    Lock_Mutex_RW lock(smutexMethods);
    if (methods.find(methodName) == methods.end() )
    {
        // Define the method:
        sRPCMethod method;
        method.reqAttribs = reqAttribs;
        method.obj = obj;
        method.rpcMethod = rpcMethod;

        // Put the method.
        methods[methodName] = method;
        return true;
    }
    return false;
}

int JAsyncRPC_Server_Methods::runRPCMethod(IAuth_Session * auth, const std::string & methodName, const Json::Value & payload, const Json::Value & extraInfo, Json::Value *payloadOut, Json::Value *extraInfoOut)
{
    // not authenticated...
    if (auth->isAuthenticated()!=AUTH_REASON_AUTHENTICATED)
        return METHOD_RET_CODE_UNAUTHENTICATED;

    Lock_Mutex_RD lock(smutexMethods);
    if (methods.find(methodName) == methods.end())
        return METHOD_RET_CODE_METHODNOTFOUND;
    else
    {
        *payloadOut = methods[methodName].rpcMethod(methods[methodName].obj, auth,payload,extraInfo,extraInfoOut);
        return METHOD_RET_CODE_SUCCESS;
    }
}

eValidationRetCodes JAsyncRPC_Server_Methods::validateRPCMethodPerms(IAuth_Session *auth, const std::string &methodName, const std::set<uint32_t> & extraTmpIndexes, Json::Value *reasons)
{
    std::set<uint32_t> passIndexesLeft;
    std::set<std::string> attribsLeft;
    Lock_Mutex_RD lock(smutexMethods);
    if (methods.find(methodName) == methods.end())
    {
        return VALIDATION_METHODNOTFOUND;
    }
    else
    {
        if (methodsAttribs.validateMethod(auth,methodName,extraTmpIndexes,&passIndexesLeft,&attribsLeft))
        {
            return VALIDATION_OK;
        }
        else
        {
            // not authorized..
            (*reasons)["passIndexesLeft"] = toValue(passIndexesLeft);
            (*reasons)["attribsLeft"] = toValue(attribsLeft);
            return VALIDATION_NOTAUTHORIZED;
        }
    }
}

IAuth_Methods_Attributes *JAsyncRPC_Server_Methods::getMethodsAttribs()
{
    return &methodsAttribs;
}

Json::Value JAsyncRPC_Server_Methods::toValue(const std::set<std::string> &t)
{
    Json::Value x;
    int v=0;
    for (const std::string & i : t)
        x[v++] = i;
    return x;
}

Json::Value JAsyncRPC_Server_Methods::toValue(const std::set<uint32_t> &t)
{
    Json::Value x;
    int v=0;
    for (const uint32_t & i : t)
        x[v++] = i;
    return x;
}

void JAsyncRPC_Server_Methods::setTimeout(const uint32_t &value)
{
    timeout = value;
}
