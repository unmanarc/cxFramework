#ifndef JASYNCRPC_HANDSHAKE_H
#define JASYNCRPC_HANDSHAKE_H

#include <cx_mem_streamparser/substreamparser.h>


enum eReqHandShakeVal {
    E_HANDSHAKE_PROTO_VERSION=0,
    E_HANDSHAKE_SERVER_INFO=1,
    E_HANDSHAKE_SERVER_VER_MAJ=2,
    E_HANDSHAKE_SERVER_VER_MIN=3,
    E_HANDSHAKE_PROD_INFO=4,
    E_HANDSHAKE_PROD_VERSION=5
};

class JAsyncRPC_Handshake : public SubStreamParser
{
public:
    JAsyncRPC_Handshake();
    bool stream(WRStatus & wrStat) override;

    void clear();

    std::string getProductInfo() const;
    void setProductInfo(const std::string &value);

    std::string getProductVersion() const;
    void setProductVersion(const std::string &value);

    std::string getServerInfo() const;
    void setServerInfo(const std::string &value);

    std::string getServerVersionMajor() const;
    void setServerVersionMajor(const std::string &value);

    std::string getServerVersionMinor() const;
    void setServerVersionMinor(const std::string &value);

    std::string getProtocolVersion() const;
    void setProtocolVersion(const std::string &value);
protected:
    ParseStatus parse() override;

private:
    std::string protocolVersion;
    std::string productInfo,productVersion,serverInfo,serverVersionMajor,serverVersionMinor;
    eReqHandShakeVal curHandShakeVal;
};

#endif // JASYNCRPC_HANDSHAKE_H
