#ifndef STREAMABLEOBJECT_H
#define STREAMABLEOBJECT_H

#include <string>
#include <stdlib.h>
#include <limits>

struct WRStatus {

    WRStatus()
    {
        bytesWritten = 0;
        succeed = true;
        finish = false;
    }

    WRStatus(const uint64_t & x)
    {
        bytesWritten = x;
        succeed = true;
        finish = false;
    }

    WRStatus & operator=(const uint64_t & x)
    {
        bytesWritten = x;
        return *this;
    }

    WRStatus & operator+=(const uint64_t & x)
    {
        bytesWritten += x;
        return *this;
    }

    WRStatus & operator+=(const WRStatus & x)
    {
        bytesWritten += x.bytesWritten;
        if (!x.succeed) succeed = false;
        if (x.finish) finish = x.finish;
        return *this;
    }

    bool succeed, finish;
    uint64_t bytesWritten;
};

/**
 * Streamable Object base class
 * This is a base class for streamable objects that can be retrieved or parsed trough read/write functions.
 */
class StreamableObject
{
public:
    StreamableObject();
    virtual ~StreamableObject();

    // TODO: what if the protocol reached std::numeric_limits<uint64_t>::max() ? enforce 64bit max. (on streamTo)
    // TODO: report percentage completed

    /**
     * @brief writeEOF proccess the end of the stream (should be implemented on streamTo)
     */
    virtual void writeEOF(bool);
    /**
     * @brief size return the size of the sub-container if it's fixed size.
     * @return std::numeric_limits<uint64_t>::max() if the stream is not fixed size
     */
    virtual uint64_t size() const { return std::numeric_limits<uint64_t>::max(); }
    virtual bool streamTo(StreamableObject * out, WRStatus & wrStatUpd)=0;
    virtual WRStatus write(const void * buf, const size_t &count, WRStatus & wrStatUpd)=0;

    WRStatus writeFullStream(const void *buf, const size_t &count, WRStatus & wrStatUpd);
    /**
     * @brief writeStream Write into stream using std::strings
     * @param buf data to be streamed.
     * @return true if succeed (all bytes written)
     */
    WRStatus writeString(const std::string & buf, WRStatus & wrStatUpd);
    WRStatus writeString(const std::string & buf);

    WRStatus strPrintf(const char * format, ...);
    WRStatus strPrintf(WRStatus & wrStatUpd, const char * format, ...);

    uint16_t getFailedWriteState() const;

protected:
    bool setFailedWriteState(const uint16_t &value = 1);
    uint16_t failedWriteState;
};

#endif // STREAMABLEOBJECT_H
