#include "httpv1_server.h"

#include <vector>

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string.hpp>

using namespace std;
using namespace boost;
using namespace boost::algorithm;

HTTPv1_Server::HTTPv1_Server(StreamableObject *sobject) : HTTPv1_Base(false, sobject)
{
    badAnswer = false;
    remotePairAddress[0]=0;
    currentParser = (SubStreamParser *)(&_clientRequest);
}

sWebFullRequest HTTPv1_Server::getFullRequest()
{
    sWebFullRequest fullReq;

    if (_clientHeaders.exist("Authorization"))
    {
        // TODO: deal with http based authorization.
    }
    if (_clientHeaders.exist("User-Agent"))
    {
        fullReq.USER_AGENT = _clientHeaders.getOptionRawStringByName("User-Agent");
    }

    fullReq.VARS_GET = getVars(HTTP_VARS_GET);
    fullReq.VARS_POST = getVars(HTTP_VARS_POST);
    fullReq.VARS_COOKIES = _clientHeaders.getOptionByName("Cookie");

    fullReq.CLIENT_IP = remotePairAddress;
    fullReq.VIRTUAL_HOST = &virtualHost;
    fullReq.VIRTUAL_PORT = &virtualPort;

    fullReq.clientHeaders = &_clientHeaders;
    fullReq.clientRequest = &_clientRequest;
    fullReq.clientContentData = &_clientContentData;

    return fullReq;
}

sWebFullResponse HTTPv1_Server::getFullResponse()
{
    sWebFullResponse fullR;

    fullR.contentData = &_serverContentData;
    fullR.headers = &_serverHeaders;
    fullR.status = &_serverCodeResponse;
    fullR.SET_COOKIES = &setCookies;

    return fullR;
}

void HTTPv1_Server::setRemotePairAddress(const char *value)
{
    strncpy(remotePairAddress,value,sizeof(remotePairAddress)-1);
}

HTTP_ContainerType HTTPv1_Server::getClientDataType()
{
    return _clientContentData.getContainerType();
}

Vars *HTTPv1_Server::getVars(const VarSource &source)
{
    switch (source)
    {
    case HTTP_VARS_POST:
        return _clientContentData.postVars();
    case HTTP_VARS_GET:
        return _clientRequest.getVarsPTR();
    }
    return nullptr;
}

void HTTPv1_Server::setServerName(const string &serverName)
{
    _serverHeaders.replace("Server", serverName);
}

bool HTTPv1_Server::processClientURI()
{
    return true;
}

bool HTTPv1_Server::processClientOptions()
{
    return true;
}

HttpRetCode HTTPv1_Server::processclientRequest()
{
    return HTTP_RET_200_OK;
}

bool HTTPv1_Server::changeToNextParser()
{
    // Server Mode:
    if (currentParser == &_clientRequest) return changeToNextParserOnClientRequest();
    else if (currentParser == &_clientHeaders) return changeToNextParserOnClientHeaders();
    else return changeToNextParserOnClientContentData();
}

bool HTTPv1_Server::changeToNextParserOnClientHeaders()
{
    // Internal checks when options are parsed (ex. check if host exist on http/1.1)
    parseHostOptions();
    prepareServerVersionOnOptions();

    if (badAnswer)
        return answer(ansBytes);
    else
    {
        uint64_t contentLength = _clientHeaders.getOptionAsUINT64("Content-Length");
        string contentType = _clientHeaders.getOptionValueStringByName("Content-Type");
       // string cookies = _clientHeaders.getOptionValueStringByName("Cookie");

        /////////////////////////////////////////////////////////////////////////////////////
        // Content-Length...
        if (contentLength)
        {
            // Content length defined.
            _clientContentData.setTransmitionMode(HTTP_CONTENT_TRANSMODE_CONTENT_LENGTH);
            if (!_clientContentData.setContentLenSize(contentLength))
            {
                // Error setting this content length size. (automatic answer)
                badAnswer = true;
                _serverCodeResponse.setRetCode(HTTP_RET_413_PAYLOAD_TOO_LARGE);
                return answer(ansBytes);
            }
            /////////////////////////////////////////////////////////////////////////////////////
            // Content-Type... (only if length is designated)
            if ( icontains(contentType,"multipart/form-data") )
            {
                _clientContentData.setContainerType(HTTP_CONTAINERTYPE_MIME);
                _clientContentData.getMultiPartVars()->setMultiPartBoundary(_clientHeaders.getOptionByName("Content-Type")->getSubVar("boundary"));
            }
            else if ( icontains(contentType,"application/x-www-form-urlencoded") )
            {
                _clientContentData.setContainerType(HTTP_CONTAINERTYPE_URL);
            }
            else
                _clientContentData.setContainerType(HTTP_CONTAINERTYPE_BIN);
            /////////////////////////////////////////////////////////////////////////////////////           
        }
        // Client cookies:
        //clientCookies.parseFromHeaders(cookies);
        // Auth:
        // TODO: deal with http based authorization.

        // Process the client header options
        if (!badAnswer)
        {
            if (!processClientOptions())
                currentParser = nullptr; // Don't continue with parsing (close the connection)
            else
            {
                // OK, we are ready.
                if (contentLength) currentParser = &_clientContentData;
                else
                {
                    // Answer here:
                    return answer(ansBytes);
                }
            }
        }
    }
    return true;
}

bool HTTPv1_Server::changeToNextParserOnClientRequest()
{
    // Internal checks when URL request has received.
    prepareServerVersionOnURI();
    if (badAnswer)
        return answer(ansBytes);
    else
    {
        if (!processClientURI())
            currentParser = nullptr; // Don't continue with parsing.
        else currentParser = &_clientHeaders;
    }
    return true;
}

bool HTTPv1_Server::changeToNextParserOnClientContentData()
{
    return answer(ansBytes);
}

bool HTTPv1_Server::streamServerHeaders(WRStatus &wrStat)
{
    // Act as a server. Send data from here.
    uint64_t strsize;

    if ((strsize=_serverContentData.getStreamSize()) == std::numeric_limits<uint64_t>::max())
    {
        // TODO: connection keep alive.
        _serverHeaders.add("Connetion", "Close");
        _serverHeaders.remove("Content-Length");
        /////////////////////
        if (_serverContentData.getTransmitionMode() == HTTP_CONTENT_TRANSMODE_CHUNKS)
            _serverHeaders.replace("Transfer-Encoding", "Chunked");
    }
    else
    {
        _serverHeaders.remove("Connetion");
        _serverHeaders.replace("Content-Length", std::to_string(strsize));
    }

    // Establish the cookies
    _serverHeaders.remove("Set-Cookie");
    setCookies.putOnHeaders(&_serverHeaders);

    return _serverHeaders.stream(wrStat);
}

void HTTPv1_Server::prepareServerVersionOnURI()
{
    _serverCodeResponse.getHttpVersion()->setVersionMajor(1);
    _serverCodeResponse.getHttpVersion()->setVersionMinor(0);

    if (_clientRequest.getHTTPVersion()->getVersionMajor()!=1)
    {
        _serverCodeResponse.setRetCode(HTTP_RET_505_HTTP_VERSION_NOT_SUPPORTED);
        badAnswer = true;
    }
    else
    {
        _serverCodeResponse.getHttpVersion()->setVersionMinor(_clientRequest.getHTTPVersion()->getVersionMinor());
    }
}

void HTTPv1_Server::prepareServerVersionOnOptions()
{
    if (_clientRequest.getHTTPVersion()->getVersionMinor()>=1)
    {
        if (virtualHost=="")
        {
            // TODO: does really need the VHost?
            _serverCodeResponse.setRetCode(HTTP_RET_400_BAD_REQUEST);
            badAnswer = true;
        }
    }
}

void HTTPv1_Server::parseHostOptions()
{
    string hostVal = _clientHeaders.getOptionValueStringByName("HOST");
    if (!hostVal.empty())
    {
        virtualPort = 80;
        vector<string> hostParts;
        split(hostParts,hostVal,is_any_of(":"),token_compress_on);
        if (hostParts.size()==1)
        {
            virtualHost = hostParts[0];
        }
        else if (hostParts.size()>1)
        {
            virtualHost = hostParts[0];
            virtualPort = (uint16_t)strtoul(hostParts[1].c_str(),nullptr,10);
        }
    }
}

bool HTTPv1_Server::answer(WRStatus &wrStat)
{
    wrStat.bytesWritten = 0;

    // Process client petition here.
    if (!badAnswer) _serverCodeResponse.setRetCode(processclientRequest());

  //  printf("@%p attending %s\n", this, _clientRequest.getURI().c_str()); fflush(stdout);

    // Answer is the last... close the connection after it.
    currentParser = nullptr;

    if (!_serverCodeResponse.stream(wrStat))
        return false;
    if (!streamServerHeaders(wrStat))
        return false;
    if (!_serverContentData.stream(wrStat))
    {
        _serverContentData.preemptiveDestroyStreamableOuput();
        return false;
    }

    // If all the data was sent OK, ret true, and destroy the external container.
    _serverContentData.preemptiveDestroyStreamableOuput();
    return true;
}

HTTP_Cookies_ServerSide *HTTPv1_Server::getSetCookies()
{
    return &setCookies;
}

WRStatus HTTPv1_Server::getAnsBytes() const
{
    return ansBytes;
}

bool HTTPv1_Server::setServerCookie(const string &cookieName, const string &cookieValue)
{
    HTTP_Cookie_Value val;
    val.setValue(cookieValue);
    return setServerCookie(cookieName,val);
}

bool HTTPv1_Server::setServerCookie(const string &cookieName, const HTTP_Cookie_Value &cookieValue)
{
    return setCookies.addCookieVal(cookieName,cookieValue);
}

string HTTPv1_Server::getClientCookie(const string &cookieName)
{
    HeaderOption * cookiesSubVars = _clientHeaders.getOptionByName("Cookie");
    if (!cookiesSubVars) return "";
    // TODO: mayus
    return cookiesSubVars->getSubVar(cookieName);
}

uint16_t HTTPv1_Server::getVirtualPort() const
{
    return virtualPort;
}

void HTTPv1_Server::setAnswerOutput(StreamableObject *outStream, bool deleteOutStream)
{
    _serverContentData.setStreamableOutput(outStream,deleteOutStream);
}

void HTTPv1_Server::setRequestInput(StreamableObject *outStream, bool deleteOutStream)
{
    _clientContentData.setStreamableOutput(outStream,deleteOutStream);
}

StreamableObject *HTTPv1_Server::output()
{
    return _serverContentData.getStreamableOuput();
}

StreamableObject *HTTPv1_Server::input()
{
    return _clientContentData.getStreamableOuput();
}

string HTTPv1_Server::getURI()
{
    return _clientRequest.getURI();
}

string HTTPv1_Server::getVirtualHost() const
{
    return virtualHost;
}
