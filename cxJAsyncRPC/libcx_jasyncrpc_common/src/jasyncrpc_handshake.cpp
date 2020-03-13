#include "jasyncrpc_handshake.h"

JAsyncRPC_Handshake::JAsyncRPC_Handshake()
{
    clear();
    setParseMode(PARSE_MODE_DELIMITER);
    setParseDelimiter("\n");
    setParseDataTargetSize(256*KB_MULT); // Max message size per line: 256K.
}

bool JAsyncRPC_Handshake::stream(WRStatus &wrStat)
{
    if (!upStream->writeString(protocolVersion,wrStat).succeed) return false;
    if (!upStream->writeString("\n",wrStat).succeed) return false;

    if (!upStream->writeString(serverInfo,wrStat).succeed) return false;
    if (!upStream->writeString("\n",wrStat).succeed) return false;

    if (!upStream->writeString(serverVersionMajor,wrStat).succeed) return false;
    if (!upStream->writeString("\n",wrStat).succeed) return false;

    if (!upStream->writeString(serverVersionMinor,wrStat).succeed) return false;
    if (!upStream->writeString("\n",wrStat).succeed) return false;

    if (!upStream->writeString(productInfo,wrStat).succeed) return false;
    if (!upStream->writeString("\n",wrStat).succeed) return false;

    if (!upStream->writeString(productVersion,wrStat).succeed) return false;
    if (!upStream->writeString("\n",wrStat).succeed) return false;

    return true;
}

void JAsyncRPC_Handshake::clear()
{
    protocolVersion = "JSON_ASYNC_RPC_1_0";
    serverInfo = "JSON Async RPC Server";
    serverVersionMajor = "1";
    serverVersionMinor = "0";

    productInfo = "Unknown Product";
    productVersion = "UNK_1_0";
    curHandShakeVal = E_HANDSHAKE_PROTO_VERSION;
}

std::string JAsyncRPC_Handshake::getProductInfo() const
{
    return productInfo;
}

void JAsyncRPC_Handshake::setProductInfo(const std::string &value)
{
    productInfo = value;
}

std::string JAsyncRPC_Handshake::getProductVersion() const
{
    return productVersion;
}

void JAsyncRPC_Handshake::setProductVersion(const std::string &value)
{
    productVersion = value;
}

std::string JAsyncRPC_Handshake::getServerInfo() const
{
    return serverInfo;
}

void JAsyncRPC_Handshake::setServerInfo(const std::string &value)
{
    serverInfo = value;
}

std::string JAsyncRPC_Handshake::getServerVersionMajor() const
{
    return serverVersionMajor;
}

void JAsyncRPC_Handshake::setServerVersionMajor(const std::string &value)
{
    serverVersionMajor = value;
}

std::string JAsyncRPC_Handshake::getServerVersionMinor() const
{
    return serverVersionMinor;
}

void JAsyncRPC_Handshake::setServerVersionMinor(const std::string &value)
{
    serverVersionMinor = value;
}

std::string JAsyncRPC_Handshake::getProtocolVersion() const
{
    return protocolVersion;
}

void JAsyncRPC_Handshake::setProtocolVersion(const std::string &value)
{
    protocolVersion = value;
}

ParseStatus JAsyncRPC_Handshake::parse()
{
    //printf("parsing data %d with string: %s\n",curHandShakeVal, getParsedData()->toString().c_str() ); fflush(stdout);
    switch(curHandShakeVal)
    {
    case E_HANDSHAKE_PROTO_VERSION:
    {
        protocolVersion = getParsedData()->toString();
        curHandShakeVal=E_HANDSHAKE_SERVER_INFO;
    } break;
    case E_HANDSHAKE_SERVER_INFO:
    {
        serverInfo = getParsedData()->toString();
        curHandShakeVal=E_HANDSHAKE_SERVER_VER_MAJ;
    } break;
    case E_HANDSHAKE_SERVER_VER_MAJ:
    {
        serverVersionMajor = getParsedData()->toString();
        curHandShakeVal=E_HANDSHAKE_SERVER_VER_MIN;
    } break;
    case E_HANDSHAKE_SERVER_VER_MIN:
    {
        serverVersionMinor = getParsedData()->toString();
        curHandShakeVal=E_HANDSHAKE_PROD_INFO;
    } break;
    case E_HANDSHAKE_PROD_INFO:
    {
        productInfo = getParsedData()->toString();
        curHandShakeVal=E_HANDSHAKE_PROD_VERSION;
    } break;
    case E_HANDSHAKE_PROD_VERSION:
    {
        productVersion = getParsedData()->toString();
        curHandShakeVal=E_HANDSHAKE_PROTO_VERSION;
        return PARSE_STAT_GOTO_NEXT_SUBPARSER;
    }
    }

    return PARSE_STAT_GET_MORE_DATA;
}
