#ifndef STREAMDECODER_URL_H
#define STREAMDECODER_URL_H

#include <cx_mem_streams/streamableobject.h>

class StreamDecoder_URL : public StreamableObject
{
public:
    StreamDecoder_URL(StreamableObject * orig);

    bool streamTo(StreamableObject *, WRStatus & ) override;
    WRStatus write(const void * buf, const size_t &count, WRStatus &wrStat) override;

    uint64_t getFinalBytesWritten() const;
    void writeEOF(bool) override;

private:
    size_t getPlainBytesSize(const unsigned char * buf, size_t count);
    WRStatus flushBytes(WRStatus &wrStat);

    inline unsigned char hex2uchar();
    inline bool isHexByte(unsigned char b);
    inline unsigned char get16Value(unsigned char byte);
    unsigned char bytes[3];
    uint8_t filled;

    uint64_t finalBytesWritten;
    StreamableObject * orig;
};

#endif // STREAMDECODER_URL_H
