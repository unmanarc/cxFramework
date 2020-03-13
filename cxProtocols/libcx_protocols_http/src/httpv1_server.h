#ifndef HTTP1SERVER_H
#define HTTP1SERVER_H

#include "httpv1_base.h"

#include "fullrequest.h"
#include "fullresponse.h"

#include "http_cookies_clientside.h"
#include "http_cookies_serverside.h"

enum VarSource
{
    HTTP_VARS_POST,
    HTTP_VARS_GET
};

class HTTPv1_Server : public HTTPv1_Base
{
public:
    HTTPv1_Server(StreamableObject * sobject);

    sWebFullRequest getFullRequest();
    sWebFullResponse getFullResponse();

    /**
     * @brief setServerTokens Set Server Header
     * @param serverTokens Server Header Product Name and Version (eg. MyLLS/5.0)
     */
    void setServerName(const std::string &serverName);
    /**
     * @brief setRemotePairAddress Internal function to set the remote pair address... (don't use)
     * @param value ip address
     */
    void setRemotePairAddress(const char * value);

    HTTP_ContainerType getClientDataType();

    Vars * getVars(const VarSource &source);

    std::string getVirtualHost() const;
    uint16_t getVirtualPort() const;

    void setAnswerOutput(StreamableObject * outStream, bool deleteOutStream = false);
    void setRequestInput(StreamableObject * outStream, bool deleteOutStream = false);

    ///////////////////////////
    // output...
    StreamableObject *output();
    StreamableObject *input();
    ///////////////////////////

    std::string getURI();

    WRStatus getAnsBytes() const;

    bool setServerCookie( const std::string &cookieName, const std::string & cookieValue);
    bool setServerCookie( const std::string &cookieName, const HTTP_Cookie_Value &cookieValue );

    std::string getClientCookie(const std::string & cookieName);

    HTTP_Cookies_ServerSide * getSetCookies();

protected:
    /**
    * @brief processClientURI Virtual function called when the Client URI request
    *                         (ex. GET / HTTP/1.1) is available.
    * @return true continue with the parsing / false end parsing and close connection.
    */
    virtual bool processClientURI();
    /**
    * @brief processClientOptions Virtual function called when the Client Option are available.
    * @return true continue with the parsing / false end parsing and close connection.
    */
    virtual bool processClientOptions();
    /**
    * @brief processClientOptions Virtual function called when the whole client request
    *                             is available (GET/Options/Post Data).
    * @return true
    */
    virtual HttpRetCode processclientRequest();

    void * getThis() override { return this; }
    bool changeToNextParser() override;

    char remotePairAddress[INET6_ADDRSTRLEN+1];

private:
    bool changeToNextParserOnClientHeaders();
    bool changeToNextParserOnClientRequest();
    bool changeToNextParserOnClientContentData();
    bool streamServerHeaders(WRStatus &wrStat);
    void prepareServerVersionOnURI();

    void prepareServerVersionOnOptions();
    void parseHostOptions();

    bool answer(WRStatus &wrStat);

    HTTP_Cookies_ServerSide setCookies;
    //HTTP_Cookies_ClientSide clientCookies;

    bool badAnswer;
    WRStatus ansBytes;
    uint16_t virtualPort;
    std::string virtualHost;
};

#endif // HTTP1SERVER_H
