#include "httpv1_client.h"

#include "helpers/base64std.h"
#include <boost/algorithm/string.hpp>

using namespace boost;
using namespace boost::algorithm;

HTTPv1_Client::HTTPv1_Client(StreamableObject *sobject) : HTTPv1_Base(true,sobject)
{
    currentParser = (SubStreamParser *)(&_serverCodeResponse);
    _clientRequest.getHTTPVersion()->setVersionMajor(1);
    _clientRequest.getHTTPVersion()->setVersionMinor(0);

    _clientRequest.setRequestMethod("GET");
}

bool HTTPv1_Client::initProtocol()
{
    WRStatus wrStat;

    if (!_clientRequest.stream(wrStat))
        return false;
    if (!streamClientHeaders(wrStat))
        return false;
    if (!_clientContentData.stream(wrStat))
        return false;

    // Succesfully initialized...
    return true;
}

bool HTTPv1_Client::changeToNextParser()
{
    if (currentParser == &_serverCodeResponse)
        currentParser = &_serverHeaders;
    else if (currentParser == &_serverHeaders)
    {
        // Process headers here:
        /////////////////////////////////////////////////////////////////////////
        // Parse server cookies...
        parseHeaders2ServerCookies();
        // Parse the transmition mode requested and act according it.
        currentParser = parseHeaders2TransmitionMode();
    }
    else // END.
        currentParser = nullptr;
    return true;
}

void HTTPv1_Client::parseHeaders2ServerCookies()
{
    std::list<HeaderOption *> setCookies = _serverHeaders.getOptionsByName("");
    for (HeaderOption * serverCookie : setCookies)
        serverCookies.parseCookie(serverCookie->getOrigValue());
}

SubStreamParser * HTTPv1_Client::parseHeaders2TransmitionMode()
{
    _serverContentData.setTransmitionMode(HTTP_CONTENT_TRANSMODE_CONNECTION_CLOSE);
    // Set Content Data Reception Mode.
    if (_serverHeaders.exist("Content-Length"))
    {
        uint64_t len = _serverHeaders.getOptionAsUINT64("Content-Length");
        _serverContentData.setTransmitionMode(HTTP_CONTENT_TRANSMODE_CONTENT_LENGTH);

        // Error setting up that size or no data... (don't continue)
        if (!len || !_serverContentData.setContentLenSize(len))
            return nullptr;
    }
    else if (icontains(_serverHeaders.getOptionValueStringByName("Transfer-Encoding"),"CHUNKED"))
        _serverContentData.setTransmitionMode(HTTP_CONTENT_TRANSMODE_CHUNKS);

    return &_serverContentData;
}

bool HTTPv1_Client::streamClientHeaders(WRStatus &wrStat)
{
    // Act as a server. Send data from here.
    uint64_t strsize;

    // Can't use chunked mode on client.
    if ((strsize=_clientContentData.getStreamSize()) == std::numeric_limits<uint64_t>::max())
        return false;
    else
    {
        _clientHeaders.remove("Connetion");
        _clientHeaders.replace("Content-Length", std::to_string(strsize));
    }

    // Put client cookies:
    clientCookies.putOnHeaders(&_clientHeaders);

    // Stream it..
    return _clientHeaders.stream(wrStat);
}

HTTP_Cookies_ServerSide * HTTPv1_Client::getServerCookies()
{
    return &serverCookies;
}

void HTTPv1_Client::setClientRequest(const std::string &hostName, const std::string &uriPath)
{
    if (!hostName.empty()) _clientRequest.getHTTPVersion()->upgradeMinorVersion(1);
    _clientRequest.setRequestURI(uriPath);
    _clientHeaders.replace("Host", hostName);
}

void HTTPv1_Client::setDontTrackFlag(bool dnt)
{
    _clientRequest.getHTTPVersion()->upgradeMinorVersion(1);
    _clientHeaders.replace("DNT", dnt?"1":"0");
}

void HTTPv1_Client::setReferer(const std::string &refererURL)
{
    _clientRequest.getHTTPVersion()->upgradeMinorVersion(1);
    _clientHeaders.replace("Referer", refererURL);
}

void HTTPv1_Client::addCookie(const std::string &cookieName, const std::string &cookieVal)
{
    clientCookies.addCookieVal(cookieName, cookieVal);
}

void HTTPv1_Client::setClientUserAgent(const std::string &userAgent)
{
    _clientHeaders.replace("User-Agent", userAgent);
}

void HTTPv1_Client::setBasicAuthorization(const std::string &user, const std::string &pass)
{
    std::string authPlainText = user + ":" + pass;
    _clientHeaders.replace("Authorization", "Basic " + b64Encode(authPlainText.c_str(), authPlainText.size()));
}
