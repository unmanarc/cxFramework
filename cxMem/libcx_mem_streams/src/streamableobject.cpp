#include "streamableobject.h"
#include <stdarg.h>
#include <stdio.h>

StreamableObject::StreamableObject()
{
    failedWriteState = 0;
}

StreamableObject::~StreamableObject()
{
}

WRStatus StreamableObject::writeFullStream(const void *buf, const size_t &count, WRStatus &wrStatUpd)
{
    WRStatus cur;
    while (cur.bytesWritten<count)
    {
        if (!(cur += write((char *)buf+cur.bytesWritten,count-cur.bytesWritten, wrStatUpd)).succeed || cur.finish)
            return cur;
    }
    return cur;
}

WRStatus StreamableObject::writeString(const std::string &buf, WRStatus &wrStatUpd)
{
    return writeFullStream(buf.c_str(), buf.size(), wrStatUpd);
}

WRStatus StreamableObject::writeString(const std::string &buf)
{
    WRStatus cur;
    return writeString(buf,cur);
}

WRStatus StreamableObject::strPrintf(const char *format, ...)
{
    WRStatus cur;
    char * var = nullptr;
    int retval;

    //////

    va_list argv;
    va_start( argv, format );
    retval = vasprintf( &var, format, argv );
    va_end( argv );

    if (retval!=-1)
        cur = writeString(var, cur);
    else
        cur.succeed=false;

    if (var) free(var);

    //////

    return cur;
}

WRStatus StreamableObject::strPrintf(WRStatus &wrStatUpd, const char *format,...)
{
    WRStatus cur;
    char * var = nullptr;
    int retval;

    //////

    va_list argv;
    va_start( argv, format );
    retval = vasprintf( &var, format, argv );
    va_end( argv );

    if (retval!=-1)
    {
        cur = writeString(var, cur);
        wrStatUpd+=cur;
    }
    else
    {
        wrStatUpd.succeed = cur.succeed = false;
    }

    if (var) free(var);

    //////

    return cur;
}

void StreamableObject::writeEOF(bool )
{

}
/*
uint64_t StreamableObject::size() const
{
  return std::numeric_limits<uint64_t>::max();
}*/


uint16_t StreamableObject::getFailedWriteState() const
{
    return failedWriteState;
}

bool StreamableObject::setFailedWriteState(const uint16_t &value)
{
    failedWriteState = value;
    return value==0;
}
