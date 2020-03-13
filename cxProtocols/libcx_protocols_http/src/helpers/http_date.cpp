#include "http_date.h"

#include <string.h>

HTTP_Date::HTTP_Date()
{
    setCurrentTime();
}

time_t HTTP_Date::getRawTime() const
{
    return rawTime;
}

void HTTP_Date::setRawTime(const time_t &value)
{
    rawTime = value;
}

std::string HTTP_Date::toString()
{
    char buffer[64];
    struct tm timeinfo;
    localtime_r(&rawTime, &timeinfo);
    strftime (buffer,80,"%a, %d %b %Y %T %Z",&timeinfo);
    return std::string(buffer);
}

bool HTTP_Date::fromString(const std::string &fTime)
{
    struct tm tm;
    memset(&tm, 0, sizeof(struct tm));
    if (strptime(fTime.c_str(), "%a, %d %b %Y %T %Z", &tm) == nullptr) return false;
    // TODO: check this function (specially with different locales).
    // TODO: check hour zones changes.
    rawTime = mktime ( &tm );
    return true;
}

void HTTP_Date::setCurrentTime()
{
    rawTime = time(nullptr);
}

void HTTP_Date::incTime(const uint32_t &seconds)
{
    rawTime += seconds;
}
