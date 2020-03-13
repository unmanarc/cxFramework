#include "jasyncrpc_fhandler.h"

JAsyncRPC_FHandler::JAsyncRPC_FHandler()
{
    execTimes = 0;
    expirationTime = 0;
}

JAsyncRPC_FHandler::~JAsyncRPC_FHandler()
{
}

void JAsyncRPC_FHandler::set(void (*asyncHandler)(void *, int, const Json::Value &, const Json::Value &), void *obj)
{
    this->asyncHandler = asyncHandler;
    this->obj = obj;
    this->expirationTime = creationTime+10;
    creationTime = time(nullptr);
    expirationTime = creationTime+5;
}

void JAsyncRPC_FHandler::exec( int retCode, const Json::Value &payload, const Json::Value &extraInfo)
{
    std::unique_lock<std::mutex> lock(mutex_exec);
    if (!execTimes++)
        asyncHandler(obj,retCode, payload, extraInfo);
}

time_t JAsyncRPC_FHandler::getCreationTime() const
{
    return creationTime;
}

void JAsyncRPC_FHandler::setExpiration(uint32_t milliseconds)
{
    expirationTime = (milliseconds/1000) + creationTime;
}

bool JAsyncRPC_FHandler::isExpired()
{
    return time(nullptr)>expirationTime;
}

void *JAsyncRPC_FHandler::getObj() const
{
    return obj;
}

void *JAsyncRPC_FHandler::getAsyncHandler() const
{
    return (void *)asyncHandler;
}

time_t JAsyncRPC_FHandler::getExpirationTime() const
{
    return expirationTime;
}

void JAsyncRPC_FHandler::setExpirationTime(const time_t &value)
{
    expirationTime = value;
}
