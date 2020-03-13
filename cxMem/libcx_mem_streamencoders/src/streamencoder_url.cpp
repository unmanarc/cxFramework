#include "streamencoder_url.h"

#include <limits>

StreamEncoder_URL::StreamEncoder_URL(StreamableObject * orig)
{
    this->orig = orig;
    finalBytesWritten =0;
}

bool StreamEncoder_URL::streamTo(StreamableObject *, WRStatus & )
{
    return false;
}

WRStatus StreamEncoder_URL::write(const void *buf, const size_t &count, WRStatus &wrStat)
{
    WRStatus cur;
    size_t pos=0;

    size_t maxStream=std::numeric_limits<size_t>::max();
    maxStream/=3;
    maxStream-=3;

    if (count>maxStream)
    {
        cur.succeed=wrStat.succeed=setFailedWriteState();
        return cur;
    }

    ///////////////////////
    while (pos<count)
    {
        size_t bytesToTransmitInPlain;
        if ((bytesToTransmitInPlain=getPlainBytesSize( ((const unsigned char *)buf)+pos,count-pos))>0)
        {
            if (!(cur+=orig->writeFullStream( ((const unsigned char *)buf)+pos ,bytesToTransmitInPlain,wrStat)).succeed)
            {
                finalBytesWritten+=cur.bytesWritten;
                return cur;
            }
            pos+=bytesToTransmitInPlain;
        }
        else
        {
            char encodedByte[8];
            snprintf(encodedByte,8, "%%%02X", *(((const unsigned char *)buf)+pos));
            if (!(cur+=orig->writeFullStream(encodedByte,3, wrStat)).succeed)
            {
                finalBytesWritten+=cur.bytesWritten;
                return cur;
            }
            pos++;
        }
    }
    finalBytesWritten+=cur.bytesWritten;
    return cur;
}

size_t StreamEncoder_URL::getPlainBytesSize(const unsigned char *buf, size_t count)
{
    for (size_t i=0;i<count;i++)
    {
        if (shouldEncodeThisByte(buf[i])) return i;
    }
    return count;
}

inline bool StreamEncoder_URL::shouldEncodeThisByte(const unsigned char &byte) const
{
    return !(
            (byte>='A' && byte<='Z') ||
            (byte>='a' && byte<='z') ||
            (byte>='0' && byte<='9')
    );
}

uint64_t StreamEncoder_URL::getFinalBytesWritten() const
{
    return finalBytesWritten;
}
