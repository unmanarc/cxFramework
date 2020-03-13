#ifndef JASYNCRPC_REQUEST_H
#define JASYNCRPC_REQUEST_H

#include <set>
#include <cx_mem_streamparser/substreamparser.h>
#include "json_streamableobject.h"
#include "jasyncrpc_authentication.h"

enum eReqProcVal {
    E_REQ_METHOD_NAME=0,
    E_REQ_METHOD_RPCMODE=1,
    E_REQ_METHOD_PAYLOAD=2,
    E_REQ_METHOD_EXTRAINFO=3,
    E_REQ_METHOD_IDS=4,
    E_REQ_METHOD_AUTH=5
};

class JAsyncRPC_Request : public SubStreamParser
{
public:
    JAsyncRPC_Request();
    bool stream(WRStatus & wrStat) override;

    void print();

    void setExtraInfo(const Json::Value & extraInfo);
    void setPayload(const Json::Value & payload);

    Json::Value getPayload();
    Json::Value getExtraInfo();

    void setMethodName(const std::string &value);
    std::string getMethodName() const;

    std::string getRpcMode() const;
    void setRpcMode(const std::string &value);

    void clear();

    void addAuthentication(const JAsyncRPC_Authentication & auth);
    void addAuthentication(uint32_t passIndex, const std::string &user, const std::string &pass);


    std::set<uint32_t> getAuthenticationsIdxs();
    JAsyncRPC_Authentication getAuthentication( const uint32_t & idx );

    uint64_t getReqId();
    void setReqId(const uint64_t &value);

    int getRetCode();
    void setRetCode(int value);

protected:
    ParseStatus parse() override;

private:
    JSON_StreamableObject payload, extraInfo, ids;
    std::string methodName, rpcMode;

    std::map<uint32_t,JAsyncRPC_Authentication> authentications;
    eReqProcVal curProcVal;
};

#endif // JASYNCRPC_REQUEST_H
