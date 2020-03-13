#ifndef STREAM_READER_H
#define STREAM_READER_H

#include <stdint.h>
#include <string>

class Stream_Reader
{
public:
    Stream_Reader();
    virtual ~Stream_Reader();
    /**
        * Read unsigned char
        * @param readOK pointer to bool variable that will be filled with the result (success or fail).
        * @return char retrived.
        * */
    unsigned char readU8(bool *readOK=nullptr);
    /**
        * Read unsigned short (16bit)
        * @param readOK pointer to bool variable that will be filled with the result (success or fail).
        * @return char retrived.
        * */
    uint16_t readU16(bool *readOK=nullptr);
    /**
        * Read unsigned integer (32bit)
        * @param readOK pointer to bool variable that will be filled with the result (success or fail).
        * @return char retrived.
        * */
    uint32_t readU32(bool *readOK=nullptr);
    /**
        * Read unsigned integer (46bit)
        * @param readOK pointer to bool variable that will be filled with the result (success or fail).
        * @return char retrived.
        * */
    uint64_t readU64(bool *readOK=nullptr);
    /**
        * Read data block of maximum of 256 bytes (teotherical).
        * @param data data to allocate the incoming bytes.
        * @param datalen data length to be allocated.
        * @param keepDataLen maximum data to be received should match the incoming data, otherwise,return false (and you should close)
        * @return true if succeed.
        */
    bool readBlock8(void * data, uint8_t datalen, bool keepDataLen = false);
    /**
        * Read data block of maximum of 64k bytes (teotherical).
        * @param data data to allocate the incoming bytes.
        * @param datalen data length to be allocated.
        * @param keepDataLen maximum data to be received should match the incoming data, otherwise,return false (and you should close)
        * @return true if succeed.
        */
    bool readBlock16(void * data, uint16_t datalen, bool keepDataLen = false);
    /**
        * Read data block of maximum of 4g bytes (teotherical).
        * @param data data to allocate the incoming bytes.
        * @param datalen data length to be allocated.
        * @param keepDataLen maximum data to be received should match the incoming data, otherwise,return false (and you should close)
        * @return true if succeed.
        */
    bool readBlock32(void * data, uint32_t datalen, bool keepDataLen = false);
    /**
        * Read and allocate a memory space data block of maximum of 4g bytes (teotherical).
        * NOTE: Allocation occurs with new [], so delete it with delete []
        * @param datalen in: maximum data length supported, out: data retrieved.
        * @return memory allocated with the retrieved data or nullptr if failed to allocate memory.
        */
    char *readBlock32WAlloc(uint32_t * datalen);
    /**
        * Read and allocate a memory space data block of maximum of 4g bytes until a delimiter bytes (teotherical).
        * @param datalen in: maximum data length supported (should be min: 65536), out: data retrieved.
        * @param delim delimiter.
        * @param delimBytes delimiter size (max: 65535 bytes).
        * @return memory allocated with the retrieved data or nullptr if failed.
        */
    char *readBlock32WAllocAndDelim(unsigned int * datalen, const char *delim, uint16_t delimBytes);
    /**
        * Read and allocate a memory space data block of maximum of 2^sizel bytes (teotherical).
        * NOTE: Allocation occurs with new [], so delete it with delete []
        * @param datalen in: maximum data length supported, out: data retrieved.
        * @return memory allocated with the retrieved data or nullptr if failed.
        */
    char *readBlockWAlloc(uint32_t * datalen, unsigned char sizel = 8);

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Null terminated strings:
    /**
        * Read a std::string of maximum 65kb.
        * @return string with the data contained inside
        */
    std::string readString(bool *readOK=nullptr, unsigned char sizel = 8);


protected:
    virtual bool readBlock(void * data, const uint32_t & datalen, uint32_t * bytesReceived = nullptr) = 0;

private:
    int32_t read64KBlockDelim(char * block, const char* delim, const uint16_t & delimBytes, const uint32_t &blockNo);
};

#endif // STREAM_READER_H
