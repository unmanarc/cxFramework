#include "jasyncrpc_request.h"

JAsyncRPC_Request::JAsyncRPC_Request()
{
    clear();
    setParseMode(PARSE_MODE_DELIMITER);
    setParseDelimiter("\n");
    setParseDataTargetSize(256*KB_MULT); // Max message size per line: 256K.

    (*ids.getValue())["reqId"] = static_cast<Json::UInt64>(0);
}

ParseStatus JAsyncRPC_Request::parse()
{
    switch(curProcVal)
    {
    case E_REQ_METHOD_NAME:
    {
        methodName = getParsedData()->toString();
        curProcVal = E_REQ_METHOD_RPCMODE;
    }break;
    case E_REQ_METHOD_RPCMODE:
    {
        rpcMode = getParsedData()->toString();
        curProcVal = E_REQ_METHOD_PAYLOAD;
    }break;
    case E_REQ_METHOD_PAYLOAD:
    {
        WRStatus stat;
        payload.clear();
        if (!getParsedData()->streamTo(&payload,stat))
            return PARSE_STAT_ERROR;
        curProcVal = E_REQ_METHOD_IDS;
    }break;
    case E_REQ_METHOD_IDS:
    {
        WRStatus stat;
        ids.clear();

        /*std::cout << "ids parsed:" << getParsedData()->toString() << std::endl << std::flush;
        std::cout << "old ids:" << ids.getString() << std::endl << std::flush;*/
        if (!getParsedData()->streamTo(&ids,stat))
            return PARSE_STAT_ERROR;
        curProcVal = E_REQ_METHOD_EXTRAINFO;
        //std::cout << "new ids:" << ids.getString() << std::endl << std::flush;

    }break;
    case E_REQ_METHOD_EXTRAINFO:
    {
        WRStatus stat;
        extraInfo.clear();
        if (!getParsedData()->streamTo(&extraInfo,stat))
            return PARSE_STAT_ERROR;
        curProcVal = E_REQ_METHOD_AUTH;
    }break;
    case E_REQ_METHOD_AUTH:
    {
        if (getParsedData()->toString().empty())
        {
            // Finished...
            curProcVal = E_REQ_METHOD_NAME;
            return PARSE_STAT_GOTO_NEXT_SUBPARSER;
        }
        else
        {
            // Authentication parsing...
            WRStatus stat;
            JSON_StreamableObject s;
            JAsyncRPC_Authentication auth;

            if (!getParsedData()->streamTo(&s,stat))
            {
               // std::cout << "failed to parse from data to json :S" << std::endl << std::flush;
                return PARSE_STAT_ERROR;
            }
            if (!auth.fromJSON((*s.getValue())))
            {
               // std::cout << "failed to parse from json" << std::endl << std::flush;
                return PARSE_STAT_ERROR;
            }

            //std::cout << "OK parsed." << std::endl << std::flush;

            addAuthentication(auth);
        }
    }break;
    }
    return PARSE_STAT_GET_MORE_DATA;
}

int JAsyncRPC_Request::getRetCode()
{
    return (*ids.getValue())["retCode"].asInt();
}

void JAsyncRPC_Request::setRetCode(int value)
{
    (*ids.getValue())["retCode"] = static_cast<Json::Int>(value);
}

uint64_t JAsyncRPC_Request::getReqId()
{
    return (*ids.getValue())["reqId"].asUInt64();
}

void JAsyncRPC_Request::setReqId(const uint64_t &value)
{
    (*ids.getValue())["reqId"] = static_cast<Json::UInt64>(value);
}

std::set<uint32_t> JAsyncRPC_Request::getAuthenticationsIdxs()
{
    std::set<uint32_t> r;
    for (const auto & i : authentications)
        r.insert(i.first);
    return r;
}

JAsyncRPC_Authentication JAsyncRPC_Request::getAuthentication(const uint32_t &idx)
{
    if (authentications.find(idx) != authentications.end())
        return authentications[idx];

    JAsyncRPC_Authentication r;
    return r;
}

bool JAsyncRPC_Request::stream(WRStatus &wrStat)
{
    /*std::cout << "Writting into channel: -------------------------------------" << std::endl << std::flush;
    print();
    std::cout << "------------------------------------------------------------" << std::endl << std::flush;*/

    if (!upStream->writeString(methodName,wrStat).succeed) return false;
    if (!upStream->writeString("\n",wrStat).succeed) return false;

    if (!upStream->writeString(rpcMode,wrStat).succeed) return false;
    if (!upStream->writeString("\n",wrStat).succeed) return false;

    if (!payload.streamTo(upStream,wrStat)) return false;
    if (!upStream->writeString("\n",wrStat).succeed) return false;

    if (!ids.streamTo(upStream,wrStat)) return false;
    if (!upStream->writeString("\n",wrStat).succeed) return false;

    if (!extraInfo.streamTo(upStream,wrStat)) return false;
    if (!upStream->writeString("\n",wrStat).succeed) return false;

    for (const auto & i : authentications)
    {
        JSON_StreamableObject s;

        s.setValue(i.second.toJSON());

        if (!s.streamTo(upStream,wrStat)) return false;
        if (!upStream->writeString("\n",wrStat).succeed) return false;
    }
    if (!upStream->writeString("\n",wrStat).succeed) return false;

    return true;
}

void JAsyncRPC_Request::print()
{
    std::cout << "REQ> Executing method: " << methodName << std::endl << std::flush;
    std::cout << "REQ> On rpc mode     : " << rpcMode << std::endl << std::flush;
    std::cout << "REQ> With payload    : " << payload.getString() <<std::flush;
    std::cout << "REQ> With id's       : " << ids.getString() <<  std::flush;
    std::cout << "REQ> With extraInfo  : " << extraInfo.getString() <<  std::flush;

    for (const auto & i : authentications)
    {
        JSON_StreamableObject s;
        s.setValue(i.second.toJSON());
        std::cout << ">>>> With auth: " << s.getString() << std::endl << std::flush;
    }
}

void JAsyncRPC_Request::setExtraInfo(const Json::Value &extraInfo)
{
    (*this->extraInfo.getValue()) = extraInfo;
}

void JAsyncRPC_Request::setPayload(const Json::Value &payload)
{
    (*this->payload.getValue()) = payload;
}

Json::Value JAsyncRPC_Request::getPayload()
{
    return (*this->payload.getValue());
}

Json::Value JAsyncRPC_Request::getExtraInfo()
{
    return (*this->extraInfo.getValue());
}


void JAsyncRPC_Request::setMethodName(const std::string &value)
{
    methodName = value;
}

std::string JAsyncRPC_Request::getMethodName() const
{
    return methodName;
}

std::string JAsyncRPC_Request::getRpcMode() const
{
    return rpcMode;
}

void JAsyncRPC_Request::setRpcMode(const std::string &value)
{
    rpcMode = value;
}

void JAsyncRPC_Request::clear()
{
    curProcVal = E_REQ_METHOD_NAME;
    rpcMode = "EXEC";
    methodName.clear();
    payload.clear();
    ids.clear();
    extraInfo.clear();
    authentications.clear();
    setReqId(0);
}

void JAsyncRPC_Request::addAuthentication(const JAsyncRPC_Authentication &auth)
{
    authentications[auth.getPassIndex()] = auth;
}

void JAsyncRPC_Request::addAuthentication(uint32_t passIndex, const std::string &user, const std::string &pass)
{
    authentications[passIndex].setPassIndex(passIndex);
    authentications[passIndex].setUserName(user);
    authentications[passIndex].setUserPass(pass);
}
