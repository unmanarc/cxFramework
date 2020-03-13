#ifndef HTTP_DATE_H
#define HTTP_DATE_H

#include <time.h>
#include <string>

class HTTP_Date
{
public:
    HTTP_Date();

    time_t getRawTime() const;
    void setRawTime(const time_t &value);

    std::string toString();
    bool fromString(const std::string & fTime);

    void setCurrentTime();
    void incTime(const uint32_t &seconds);

private:
    time_t rawTime;
};

#endif // HTTP_DATE_H
