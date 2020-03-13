#include "streamdecoder_url.h"

StreamDecoder_URL::StreamDecoder_URL(StreamableObject * orig)
{
    this->orig = orig;
    filled=0;
    finalBytesWritten =0;
}

bool StreamDecoder_URL::streamTo(StreamableObject *, WRStatus & )
{
    return false;
}

WRStatus StreamDecoder_URL::write(const void *buf, const size_t &count, WRStatus &wrStat)
{
    WRStatus cur;
    size_t pos=0;

    while (pos<count)
    {
        switch (filled)
        {
        case 0:
        {
            size_t bytesToTransmitInPlain;
            if ((bytesToTransmitInPlain=getPlainBytesSize(((unsigned char *)buf)+pos,count-pos))>0)
            {
                if (!(cur+=orig->writeFullStream(((unsigned char *)buf)+pos,bytesToTransmitInPlain,wrStat)).succeed)
                {
                    finalBytesWritten+=cur.bytesWritten;
                    return cur;
                }
                pos+=bytesToTransmitInPlain;
            }
            else
            {
                bytes[0]='%';
                pos++;
                filled = 1;
            }
        }break;
        case 1:
        {
            bytes[1]=*(((unsigned char *)buf)+pos);
            pos++;
            filled = 2;
            if (!isHexByte(bytes[1]))
            {
                // Write original 2 bytes...
                if (!(cur+=flushBytes(wrStat)).succeed)
                {
                    finalBytesWritten+=cur.bytesWritten;
                    return cur;
                }
            }
        }break;
        case 2:
        {
            bytes[2]=*(((unsigned char *)buf)+pos);
            filled = 0;
            pos++;
            if (!isHexByte(bytes[2]))
            {
                // Write original 2 bytes...
                if (!(cur+=flushBytes(wrStat)).succeed)
                {
                    finalBytesWritten+=cur.bytesWritten;
                    return cur;
                }
            }
            unsigned char val[2];
            val[0] = hex2uchar();
            if (!(cur+=orig->writeFullStream(val,1, wrStat)).succeed)
            {
                finalBytesWritten+=cur.bytesWritten;
                return cur;
            }
        }break;
        default:
            break;
        }
    }
    finalBytesWritten+=cur.bytesWritten;
    return cur;
}

size_t StreamDecoder_URL::getPlainBytesSize(const unsigned char *buf, size_t count)
{
    for (size_t i=0;i<count;i++)
    {
        if (buf[i]=='%') return i;
    }
    return count;
}

WRStatus StreamDecoder_URL::flushBytes(WRStatus & wrStat)
{
    return orig->writeFullStream(bytes,filled, wrStat);
}

inline unsigned char StreamDecoder_URL::hex2uchar()
{
    return get16Value(bytes[1])*0x10+get16Value(bytes[2]);
}

inline bool StreamDecoder_URL::isHexByte(unsigned char byte)
{
    return  (byte>='A' && byte<='F') ||
            (byte>='a' && byte<='f') ||
            (byte>='0' && byte<='9');
}

inline unsigned char StreamDecoder_URL::get16Value(unsigned char byte)
{
    if (byte>='A' && byte<='F') return byte-'A'+10;
    else if (byte>='a' && byte<='f') return byte-'a'+10;
    else if (byte>='0' && byte<='9') return byte-'0';
    return 0;
}

uint64_t StreamDecoder_URL::getFinalBytesWritten() const
{
    return finalBytesWritten;
}

void StreamDecoder_URL::writeEOF(bool )
{
    // flush intermediary bytes...
    WRStatus w;
    flushBytes(w);
}
