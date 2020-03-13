#ifndef STREAM_WRITER_H
#define STREAM_WRITER_H

#include <stdint.h>
#include <string>

class Stream_Writer
{
public:
    Stream_Writer();
    virtual ~Stream_Writer();
    /**
         * Write unsigned char
         * @param c char to write into the socket.
         * @return true if succeed.
         * */
    bool writeU8(const unsigned char & c);
    /**
         * Write unsigned short (16bit)
         * @param c char to write into the socket.
         * @return true if succeed.
         * */
    bool writeU16(const uint16_t & c);
    /**
         * Write unsigned integer (32bit)
         * @param c char to write into the socket.
         * @return true if succeed.
         * */
    bool writeU32(const uint32_t & c);
    /**
         * Write unsigned integer (64bit)
         * @param c char to write into the socket.
         * @return true if succeed.
         * */
    bool writeU64(const uint64_t & c);
    /**
         * Write data block of maximum of 4g bytes (teotherical).
         * @param data data block bytes
         * @param datalen data length to be sent.
         * @return true if succeed.
         */
    bool writeBlock32(const void * data, const uint32_t & datalen);
    /**
         * Write data block of maximum of 64k bytes (teotherical).
         * @param data data block bytes
         * @param datalen data length to be sent.
         * @return true if succeed.
         */
    bool writeBlock16(const void * data, const uint16_t &datalen);
    /**
         * Write data block of maximum of 256 bytes (teotherical).
         * @param data data block bytes
         * @param datalen data length to be sent.
         * @return true if succeed.
         */
    bool writeBlock8(const void * data, const uint8_t & datalen);
    /**
         * Write a std::string of maximum 4Gb.
         * @param str string to be sent.
         * @return true if success
         */
    bool writeString32(const std::string & str, uint32_t maxSize);
    /**
         * Write a std::string of maximum 65kb.
         * @param str string to be sent.
         * @return true if success
         */
    bool writeString16(const std::string & str);
    /**
         * Write a std::string of maximum 256 bytes.
         * @param str string to be sent.
         * @return true if success
         */
    bool writeString8(const std::string & str);

protected:
    virtual bool writeBlock(const void * data, const uint32_t & datalen) = 0;
};

#endif // STREAM_WRITER_H
