#ifndef HTTP1BASE_H
#define HTTP1BASE_H

#include <cx_mem_streamparser/streamparser.h>
#include <cx_mem_vars/vars.h>

#include "http_request.h"
#include "http_content.h"
#include "http_status.h"
#include <cx_protocols_mime/mime_sub_header.h>

#include <netinet/in.h>

#define VNET_PROD_NAME "vProtonHTTP"

#define VNET_HTTP_VERSION_MAJOR 0
#define VNET_HTTP_VERSION_MINOR 1

struct HTTPServerParams
{
    HTTP_Status * status;
    HTTP_Content * content;
    MIME_Sub_Header * headers;
};

struct HTTPClientParams
{
    HTTP_Request * request;
    HTTP_Content * content;
    MIME_Sub_Header * headers;
};


class HTTPv1_Base : public StreamParser
{
public:
    HTTPv1_Base(bool clientMode, StreamableObject *sobject);
    virtual ~HTTPv1_Base()  override {}

    // httpElements:
    HTTPServerParams server();
    HTTPClientParams client();

protected:
    virtual bool initProtocol() override;
    virtual void endProtocol() override;

    virtual void * getThis()=0;
    virtual bool changeToNextParser()  override = 0;
    /**
     * @brief code Response - Server code response. (HTTP Version, Response code, message)
     */
    HTTP_Status _serverCodeResponse;
    /**
     * @brief headers - Options Values.
     */
    MIME_Sub_Header _clientHeaders;
    /**
     * @brief content - Content Data.
     */
    HTTP_Content _clientContentData;
    /**
     * @brief clientRequest - URL Request (Request type, URL, GET Vars, and HTTP version)
     */
    HTTP_Request _clientRequest;
    /**
     * @brief headers - Options Values.
     */
    MIME_Sub_Header _serverHeaders;
    /**
     * @brief content - Content Data.
     */
    HTTP_Content _serverContentData;

private:
    void setInternalProductVersion(const std::string & prodName, const std::string & extraInfo, const uint32_t &versionMajor = VNET_HTTP_VERSION_MAJOR, const uint32_t &versionMinor = VNET_HTTP_VERSION_MINOR);

};

#endif // HTTP1BASE_H
