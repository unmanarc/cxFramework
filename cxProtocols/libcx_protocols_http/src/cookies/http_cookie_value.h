#ifndef HTTP_COOKIE_VALUE_H
#define HTTP_COOKIE_VALUE_H

#include <string>
#include "http_date.h"

class HTTP_Cookie_Value
{
public:
    HTTP_Cookie_Value();

    std::string toSetCookieString(const std::string & cookieName);
    bool fromSetCookieString(const std::string & setCookieValue, std::string * cookieName);

    ///////////////////////////////////////////////////////////////////////////////
    time_t getExpiration() const;
    void setExpiration(const time_t& exp);
    void setExpirationInSeconds(const uint32_t& seconds);

    std::string getValue() const;
    void setValue(const std::string &value);

    uint32_t getMaxAge() const;
    void setMaxAge(const uint32_t &value);

    std::string getDomain() const;
    void setDomain(const std::string &value);

    std::string getPath() const;
    void setPath(const std::string &value);

    bool isHttpOnly() const;
    void setHttpOnly(bool value);

    bool isSecure() const;
    void setSecure(bool value);

    std::string getSameSite() const;
    void isSameSite(const std::string &value);

private:
    std::pair<std::string, std::string> getVarNameAndValue(const std::string &var);

    std::string value;

    HTTP_Date expires;
    uint32_t max_age;
    std::string domain;
    std::string path;
    bool secure,httpOnly;
    std::string sameSite;
};

#endif // HTTP_COOKIE_VALUE_H
