#include "http_cookie_value.h"

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string.hpp>

#include <vector>

using namespace std;
using namespace boost;
using namespace boost::algorithm;

HTTP_Cookie_Value::HTTP_Cookie_Value()
{
    // Don't expire.
    expires.setRawTime(0);
    max_age=0;
    secure=false;
    httpOnly=false;
}

std::string HTTP_Cookie_Value::toSetCookieString(const std::string &cookieName)
{
    std::string opts = cookieName + "=" + value + "; ";

    if (expires.getRawTime()) opts+= "Expires=" + expires.toString() + "; ";
    if (max_age) opts+= "Max-Age=" + std::to_string(max_age) + "; ";
    if (secure) opts+= "Secure; ";
    if (httpOnly) opts+= "HttpOnly; ";
    if (!domain.empty()) opts+= "Domain=" + domain + "; ";
    if (!path.empty()) opts+= "Path=" + path + "; ";
    if (!sameSite.empty()) opts+= "SameSite=" + sameSite + "; ";

    return opts;
}

bool HTTP_Cookie_Value::fromSetCookieString(const std::string &setCookieValue, string *cookieName)
{
    vector<string> cookiesParams;
    split(cookiesParams,setCookieValue,is_any_of(";"),token_compress_on);

    bool firstVal = true;
    for (const string & param : cookiesParams)
    {
        std::pair<string, string> var = getVarNameAndValue(param);

        if (firstVal)
        {
            firstVal=false;
            // Get Value and
            *cookieName = var.first;
            value = var.second;
        }
        else
        {
            std::string varNameUpper = boost::to_upper_copy(var.first);

            if      (varNameUpper == "EXPIRES") expires.fromString(var.second);
            else if (varNameUpper == "MAX-AGE") max_age = strtoul(var.second.c_str(),nullptr,10);
            else if (varNameUpper == "SECURE") secure = true;
            else if (varNameUpper == "HTTPONLY") httpOnly = true;
            else if (varNameUpper == "DOMAIN") domain = var.second;
            else if (varNameUpper == "PATH") path = var.second;
            else if (varNameUpper == "SAMESITE") sameSite = var.second;
            else
            {
                // Unknown option...
            }
        }
    }

    return true;
}

time_t HTTP_Cookie_Value::getExpiration() const
{
    return expires.getRawTime();
}

void HTTP_Cookie_Value::setExpiration(const time_t &exp)
{
    expires.setRawTime(exp);
}

void HTTP_Cookie_Value::setExpirationInSeconds(const uint32_t &seconds)
{
    expires.setCurrentTime();
    expires.incTime(seconds);
}

std::string HTTP_Cookie_Value::getValue() const
{
    return value;
}

void HTTP_Cookie_Value::setValue(const std::string &value)
{
    this->value = value;
}

uint32_t HTTP_Cookie_Value::getMaxAge() const
{
    return max_age;
}

void HTTP_Cookie_Value::setMaxAge(const uint32_t &value)
{
    max_age = value;
}

std::string HTTP_Cookie_Value::getDomain() const
{
    return domain;
}

void HTTP_Cookie_Value::setDomain(const std::string &value)
{
    domain = value;
}

std::string HTTP_Cookie_Value::getPath() const
{
    return path;
}

void HTTP_Cookie_Value::setPath(const std::string &value)
{
    path = value;
}

bool HTTP_Cookie_Value::isHttpOnly() const
{
    return httpOnly;
}

void HTTP_Cookie_Value::setHttpOnly(bool value)
{
    httpOnly = value;
}

bool HTTP_Cookie_Value::isSecure() const
{
    return secure;
}

void HTTP_Cookie_Value::setSecure(bool value)
{
    secure = value;
}

std::string HTTP_Cookie_Value::getSameSite() const
{
    return sameSite;
}

void HTTP_Cookie_Value::isSameSite(const std::string &value)
{
    sameSite = value;
}

std::pair<string, string> HTTP_Cookie_Value::getVarNameAndValue(const string &var)
{
    std::pair<string, string> r;

    size_t found=var.find("=");

    if (found!=std::string::npos)
    {
        // We have parameters..
        r.second = var.c_str()+found+1;
        r.first = std::string(var.c_str(),found);
    }
    else
    {
        // Empty option...
        r.first = var;
    }

    trim(r.first);
    trim(r.second);

    return r;
}
