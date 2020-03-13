#include "jasyncrpc_authentication.h"

JAsyncRPC_Authentication::JAsyncRPC_Authentication()
{
    passIndex = 0;
}

JAsyncRPC_Authentication::JAsyncRPC_Authentication(const std::string &user, const std::string &pass, const uint32_t &idx)
{
    userName = user;
    userPass = pass;
    passIndex = idx;
}

bool JAsyncRPC_Authentication::fromJSON(const Json::Value &x)
{
    if (!x["user"].isNull()) userName = x["user"].asString();
    else return false;
    if (!x["pass"].isNull()) userPass = x["pass"].asString();
    else return false;
    if (!x["idx"].isNull()) passIndex = x["idx"].asUInt();
    else return false;
    return true;
}

Json::Value JAsyncRPC_Authentication::toJSON() const
{
    Json::Value x;
    x["user"] = userName;
    x["pass"] = userPass;
    x["idx"] = passIndex;
    return x;
}

std::string JAsyncRPC_Authentication::getUserName() const
{
    return userName;
}

void JAsyncRPC_Authentication::setUserName(const std::string &value)
{
    userName = value;
}

std::string JAsyncRPC_Authentication::getUserPass() const
{
    return userPass;
}

void JAsyncRPC_Authentication::setUserPass(const std::string &value)
{
    userPass = value;
}

uint32_t JAsyncRPC_Authentication::getPassIndex() const
{
    return passIndex;
}

void JAsyncRPC_Authentication::setPassIndex(const uint32_t &value)
{
    passIndex = value;
}
