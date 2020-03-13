#ifndef JASYNCRPC_SERVER_METHODS_H
#define JASYNCRPC_SERVER_METHODS_H

#include <map>
#include <list>
#include <json/json.h>
#include <cx_auth/iauth.h>

#include <cx_auth/iauth_validation_methods.h>
#include <cx_thr_threadpool/threadpool.h>
#include <cx_thr_mutex/lock_mutex_shared.h>

#include "jasyncrpc_validation_retcodes.h"

struct sRPCParameters
{
    void * rpcMethodsCaller;
    void * connectionSender;
    IAuth_Session *auth;
    std::string methodName;
    Json::Value payload;
    Json::Value extraInfo;
    uint64_t requestId;
};

struct sRPCMethod
{
    /**
     * @brief obj object to pass
     */
    void * obj;
    std::list<std::string> reqAttribs;
    Json::Value (*rpcMethod)(void * obj, IAuth_Session * auth, const Json::Value & parameters, const Json::Value & extraInfo, Json::Value * extraInfoOut);
};

class JAsyncRPC_Server_Methods
{
public:
    JAsyncRPC_Server_Methods(uint32_t threadsCount = 52, uint32_t taskQueues = 36);
    ~JAsyncRPC_Server_Methods();

    void stop();

    /**
     * @brief setTimeout Timeout in milliseconds to desist to put the execution task
     * @param value milliseconds
     */
    void setTimeout(const uint32_t &value);
    /**
     * @brief pushRPCMethodIntoQueue push a received order in the task queue
     * @param params RPC parameters
     * @param key Key to be prioritized
     * @param priority priority (0-1] to use n-queues
     * @return true if inserted, false if failed (saturated)
     */
    bool pushRPCMethodIntoQueue( sRPCParameters * params, const std::string & key, const float & priority=0.5 );

    //////////////////////////////////////////////////
    /**
     * @brief addRPCMethod
     * @param methodName
     * @param obj
     * @return
     */
    bool addRPCMethod(const std::string & methodName, const std::list<std::string> & reqAttribs, Json::Value (*rpcMethod)(void *, IAuth_Session *, const Json::Value &,const Json::Value &, Json::Value *), void * obj);
    /**
     * @brief runRPCMethod2
     * @param methodName
     * @param parsedParams
     * @param extraInfo
     * @param answer
     * @return 0 if succeed, -4 if method not found.
     */
    int runRPCMethod(IAuth_Session *auth, const std::string & methodName, const Json::Value & payload, const Json::Value & extraInfo, Json::Value *payloadOut, Json::Value *extraInfoOut);
    /**
     * @brief validateRPCMethod
     * @param auth
     * @param methodName
     * @param payloadOut
     * @param extraInfoOut
     * @return
     */
    eValidationRetCodes validateRPCMethodPerms(IAuth_Session *auth, const std::string & methodName, const std::set<uint32_t> &extraTmpIndexes, Json::Value * reasons);

    /**
     * @brief getMethodsAttribs Use for method initialization only.
     * @return methods required attributes
     */
    IAuth_Methods_Attributes * getMethodsAttribs();


private:
    Json::Value toValue(const std::set<std::string> &t);
    Json::Value toValue(const std::set<uint32_t> &t);

    // Methods:
    // method name -> method.
    std::map<std::string,sRPCMethod> methods;

    IAuth_Methods_Attributes methodsAttribs;

    ThreadPool * threadPool;
    // lock for methods manipulation...
    Mutex_RW smutexMethods;

    std::atomic<uint32_t> timeout;
};

#endif // JASYNCRPC_SERVER_METHODS_H
