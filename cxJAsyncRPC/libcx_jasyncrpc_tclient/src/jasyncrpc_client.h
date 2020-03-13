#ifndef JASYNCRPC_CLIENT_H
#define JASYNCRPC_CLIENT_H

#include <cx_mem_streamparser/streamparser.h>
#include <cx_auth/iauth.h>
#include <json/json.h>

#include <cx_jasyncrpc_common/jasyncrpc_authentication.h>
#include <cx_jasyncrpc_common/jasyncrpc_request.h>
#include <cx_jasyncrpc_common/jasyncrpc_handshake.h>

#include <cx_thr_mutex_map/mutex_map.h>
#include <cx_thr_threads/garbagecollector.h>

#include <cx_thr_threads/threaded.h>
#include "retinfo.h"

#include <mutex>
#include <condition_variable>

class JAsyncRPC_Client : public StreamParser
{
public:
    JAsyncRPC_Client( StreamableObject *sobject );
    ~JAsyncRPC_Client() override;

    ////////////////////////////////////////////////////////////////////
    // Threading:
    void garbageCollector();
    void waitForEmpty();
    void start();

    ////////////////////////////////////////////////////////////////////
    // Authentication:
    AuthReason authenticate(const std::string & user, const std::string &pass, const uint32_t &index = 0);
    AuthReason authenticate( const JAsyncRPC_Authentication & authData );

    ////////////////////////////////////////////////////////////////////
    // execution:
    std::pair<bool,uint64_t> execASync(const std::string & methodName, const Json::Value & payload, std::list<JAsyncRPC_Authentication> extraAuths, const Json::Value & extraInfo = "",
                   uint32_t maxWaitTimeMilliSeconds = 10000,
                   void (*asyncHandler)(void *, int, const Json::Value &,const Json::Value &) = nullptr, void * obj = nullptr);

    int execSync(const std::string & methodName, const Json::Value & payload, std::list<JAsyncRPC_Authentication> extraAuths,
                 sRetInfo *retInfo, const Json::Value & extraInfo = "", uint32_t maxWaitTimeMilliSeconds = 10000);

    ////////////////////////////////////////////////////////////////////
    // channels:
    // TODO: channels:
    /*void registerOnChannel( const std::string & channelName, std::list<JAsyncRPC_Authentication> extraAuths,
                            void (*channelHandler)(void *, const Json::Value &), void * obj = nullptr);*/

    ////////////////////////////////////////////////////////////////////
    // Getters/Setters.
    JAsyncRPC_Handshake *getLocalHandshake();
    JAsyncRPC_Handshake *getRemoteHandshake();

    ////////////////////////////////////////////////////////////////////
    // wait for valid handshake...
    bool waitForValidHandShake(uint32_t maxWaitTimeMilliSeconds = 1000);

    uint64_t getExpiredElements() const;

protected:
    bool initProtocol() override;
    void endProtocol() override {}
    void * getThis() { return this; }
    bool changeToNextParser() override;

private:
    bool processAnswer();
    bool sendHandshake();
    bool sendClientRequest();
    bool validateHandshake();

    JAsyncRPC_Handshake localHandshake, remoteHandshake;

    std::mutex mutex_validHandShake, mutex_execHandlersMap;
    std::timed_mutex mutex_writeData;
    std::condition_variable_any cond_loginAnswered, cond_validHandShake, cond_execHandlersMapEmpty;
    int isValidHandShake;

    Mutex_Map<uint64_t> execHandlersMap;
    std::atomic<uint64_t> curExecutionId, expiredElements;
    Json::Value authAnswer;
    JAsyncRPC_Request serverAnswer,clientRequest;
    Threaded thread;
    GarbageCollector gc;
};

#endif // JASYNCRPC_CLIENT_H
