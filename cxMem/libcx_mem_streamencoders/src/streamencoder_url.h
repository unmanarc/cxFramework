#ifndef STREAMENCODER_URL_H
#define STREAMENCODER_URL_H

#include <cx_mem_streams/streamableobject.h>

class StreamEncoder_URL : public StreamableObject
{
public:
    StreamEncoder_URL(StreamableObject * orig);
    bool streamTo(StreamableObject *, WRStatus & ) override;
    WRStatus write(const void * buf, const size_t &count, WRStatus &wrStat) override;
    uint64_t getFinalBytesWritten() const;

private:
    size_t getPlainBytesSize(const unsigned char * buf, size_t count);
    inline bool shouldEncodeThisByte(const unsigned char & byte) const;

    uint64_t finalBytesWritten;
    StreamableObject * orig;
};

#endif // STREAMENCODER_URL_H
