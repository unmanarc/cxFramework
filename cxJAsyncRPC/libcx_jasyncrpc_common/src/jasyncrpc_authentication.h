#ifndef JASYNCRPC_AUTHENTICATION_H
#define JASYNCRPC_AUTHENTICATION_H

#include <string>
#include <json/json.h>

class JAsyncRPC_Authentication
{
public:
    JAsyncRPC_Authentication();
    JAsyncRPC_Authentication(const std::string & user, const std::string & pass, const uint32_t & idx);

    bool fromJSON( const Json::Value & x );
    Json::Value toJSON() const;

    std::string getUserName() const;
    void setUserName(const std::string &value);

    std::string getUserPass() const;
    void setUserPass(const std::string &value);

    uint32_t getPassIndex() const;
    void setPassIndex(const uint32_t &value);

private:
    std::string userName,userPass;
    uint32_t passIndex;
};

#endif // JASYNCRPC_AUTHENTICATION_H
