#include "http_cookies_serverside.h"

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string.hpp>

#include <vector>
#include <string>

using namespace std;
using namespace boost;
using namespace boost::algorithm;

HTTP_Cookies_ServerSide::HTTP_Cookies_ServerSide()
{
}

HTTP_Cookies_ServerSide::~HTTP_Cookies_ServerSide()
{
    for (auto & cookie : cookiesMap) delete cookie.second;
}

void HTTP_Cookies_ServerSide::putOnHeaders(MIME_Sub_Header *headers) const
{
    for (const auto & cookie :cookiesMap )
    {
        headers->add("Set-Cookie",((HTTP_Cookie_Value *)cookie.second)->toSetCookieString(cookie.first));
    }
}

string HTTP_Cookies_ServerSide::getCookieValueByName(const string &cookieName)
{
    HTTP_Cookie_Value * cookieValue = getCookieByName(cookieName);
    return !cookieValue?"":cookieValue->getValue();
}

HTTP_Cookie_Value * HTTP_Cookies_ServerSide::getCookieByName(const string &cookieName)
{
    if (cookiesMap.find(cookieName) == cookiesMap.end()) return nullptr;
    return cookiesMap[cookieName];
}

bool HTTP_Cookies_ServerSide::parseCookie(const string &cookie_str)
{
    std::string cookieName;
    HTTP_Cookie_Value * cookieValue = new HTTP_Cookie_Value;
    cookieValue->fromSetCookieString(cookie_str,&cookieName);
    if (cookieName.empty() || cookiesMap.find(cookieName) != cookiesMap.end())
    {
        delete cookieValue;
        return false;
    }
    else
    {
        cookiesMap[cookieName] = cookieValue;
        return true;
    }
}

bool HTTP_Cookies_ServerSide::addCookieVal(const string &cookieName, const HTTP_Cookie_Value &cookieValue)
{
    if (cookiesMap.find(cookieName) != cookiesMap.end()) return false;

    HTTP_Cookie_Value * val = new HTTP_Cookie_Value;
    *val = cookieValue;

    cookiesMap[cookieName] = val;

    return true;
}

void HTTP_Cookies_ServerSide::addClearCookie(const string &cookieName)
{
    HTTP_Cookie_Value c;
    cookiesMap.erase(cookieName);
    addCookieVal(cookieName,c);
}
