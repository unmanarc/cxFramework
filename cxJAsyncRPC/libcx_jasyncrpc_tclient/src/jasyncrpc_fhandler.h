#ifndef JASYNCRPC_FHANDLER_H
#define JASYNCRPC_FHANDLER_H


#include <json/json.h>
#include <cx_thr_mutex_map/mutex_map_element.h>
#include <cx_thr_mutex/mutex.h>

class JAsyncRPC_FHandler : public Mutex_Map_Element
{
public:
    JAsyncRPC_FHandler();
    ~JAsyncRPC_FHandler();

    void set(void (*asyncHandler)(void *, int, const Json::Value &,const Json::Value &) = nullptr, void * obj = nullptr);
    void exec(int retCode, const Json::Value & payload, const Json::Value & extraInfo);

    time_t getCreationTime() const;
    void setExpiration(uint32_t milliseconds);
    bool isExpired();

    void *getObj() const;
    void * getAsyncHandler() const;
    time_t getExpirationTime() const;

    void setExpirationTime(const time_t &value);

private:
    void (*asyncHandler)(void *, int, const Json::Value &,const Json::Value &);
    void * obj;
    int execTimes;
    Mutex mutex_exec;
    time_t creationTime;
    time_t expirationTime;
};

#endif // JASYNCRPC_FHANDLER_H
