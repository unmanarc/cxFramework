#ifndef HTTP_URL_REQUEST_H
#define HTTP_URL_REQUEST_H

#include <cx_mem_streamparser/substreamparser.h>
#include <cx_mem_containers/binarycontainer_base.h>

#include <string>
#include "http_version.h"

#include <cx_protocols_urlvars/url_vars.h>

class HTTP_Request : public SubStreamParser
{
public:
    HTTP_Request();
    ////////////////////////////////////////////////
    // Virtuals:
    /**
     * @brief writeInStream Write in Stream declaration from virtual on ProtocolSubParser_Base
     *        writes this class objects/data into the http uplink.
     * @return true if written successfully/
     */
    bool stream(WRStatus & wrStat) override;

    ////////////////////////////////////////////////
    // Objects:
    /**
     * @brief getHttpVersion Get HTTP Version requested.
     * @return version object requested.
     */
    HTTP_Version * getHTTPVersion();
    /**
     * @brief getGETVars Get object that handles HTTP Vars
     * @return object that handles http vars.
     */
    Vars * getVarsPTR();

    //////////////////////////////////////////////////
    // Local getters/setters.
    /**
     * @brief Get Request Method (GET/POST/HEAD/...)
     * @return request method string.
     */
    std::string getRequestMethod() const;
    void setRequestMethod(const std::string &value);

    std::string getURI() const;
    void setRequestURI(const std::string &value);

    //////////////////////////////////////////////////
    // Security:
    void setSecurityMaxURLSize(size_t value);

    std::string getRequestURIParameters() const;

protected:
    ParseStatus parse() override;

private:
    void parseURI();
    void parseGETParameters();

    /**
     * @brief requestMethod - method requested: GET/POST/HEAD/...
     */
    std::string requestMethod;
    /**
     * @brief requestURL - URL Requested (without vars) E.g. /index.html
     */
    std::string requestURI;
    /**
     * @brief request URI Parameters
     */
    std::string requestURIParameters;

    HTTP_Version httpVersion;
    URL_Vars getVars;
};

#endif // HTTP_URL_REQUEST_H
