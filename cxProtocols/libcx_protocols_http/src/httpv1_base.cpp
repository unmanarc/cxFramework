#include "httpv1_base.h"

HTTPv1_Base::HTTPv1_Base(bool clientMode, StreamableObject *sobject) : StreamParser(sobject,clientMode)
{
    initSubParser(&_clientRequest);
    initSubParser(&_clientHeaders);
    initSubParser(&_clientContentData);

    initSubParser(&_serverCodeResponse);
    initSubParser(&_serverHeaders);
    initSubParser(&_serverContentData);

    setInternalProductVersion("vProtonHTTP_","(+http://vlibs.unmanarc.com/)");
}

HTTPServerParams HTTPv1_Base::server()
{
    HTTPServerParams ret;
    ret.status = &_serverCodeResponse;
    ret.content = &_serverContentData;
    ret.headers = &_serverHeaders;
    return ret;
}

HTTPClientParams HTTPv1_Base::client()
{
    HTTPClientParams ret;
    ret.content = &_clientContentData;
    ret.headers = &_clientHeaders;
    ret.request = &_clientRequest;
    return ret;
}

bool HTTPv1_Base::initProtocol()
{
    return true;
}

void HTTPv1_Base::endProtocol()
{

}

void HTTPv1_Base::setInternalProductVersion(const std::string &prodName, const std::string &extraInfo, const uint32_t &versionMajor, const uint32_t &versionMinor)
{
    _serverHeaders .replace("Server",
                prodName + "/" + std::to_string(versionMajor) + "." + std::to_string(versionMinor) +
                (!extraInfo.empty()? (" " + extraInfo) :"") );
}
