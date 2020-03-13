#include "stream_reader.h"
#ifdef _WIN32
#include <Winsock2.h>
#else
#include <arpa/inet.h>
#endif
#include <string.h>
#include <ctgmath>

#if __BIG_ENDIAN__
# define htonll(x) (x)
# define ntohll(x) (x)
#else
# define htonll(x) (((uint64_t)htonl((x) & 0xFFFFFFFF) << 32) | htonl((x) >> 32))
# define ntohll(x) (((uint64_t)ntohl((x) & 0xFFFFFFFF) << 32) | ntohl((x) >> 32))
#endif

Stream_Reader::Stream_Reader()
{

}

Stream_Reader::~Stream_Reader()
{

}

unsigned char Stream_Reader::readU8(bool* readOK)
{
    unsigned char rsp[1] =
    { 0 };
    if (readOK)
        *readOK = true;
    // Receive 1 byte, if fails, readOK is setted as false.
    uint32_t r;
    if ((!readBlock(&rsp, 1, &r) || r!=1) && readOK)
        *readOK = false;
    return rsp[0];
}


uint16_t Stream_Reader::readU16(bool* readOK)
{
    uint16_t ret = 0;
    if (readOK)
        *readOK = true;
    // Receive 2 bytes (unsigned short), if fails, readOK is setted as false.
    uint32_t r;
    if ((!readBlock(&ret, sizeof(uint16_t), &r) || r!=sizeof(uint16_t)) && readOK)
        *readOK = false;
    ret = ntohs(ret); // Reconvert into host based integer.
    return ret;
}


uint32_t Stream_Reader::readU32(bool* readOK)
{
    uint32_t ret = 0;
    if (readOK)
        *readOK = true;
    // Receive 4 bytes (unsigned int), if fails, readOK is setted as false.

    uint32_t r;
    if ((!readBlock(&ret, sizeof(uint32_t), &r) || r!=sizeof(uint32_t)) && readOK)
        *readOK = false;
    ret = ntohl(ret); // Reconvert into host based integer.
    return ret;
}

uint64_t Stream_Reader::readU64(bool *readOK)
{
    uint64_t ret = 0;
    if (readOK)
        *readOK = true;
    // Receive 4 bytes (unsigned int), if fails, readOK is setted as false.

    uint32_t r;
    if ((!readBlock(&ret, sizeof(uint64_t), &r) || r!=sizeof(uint64_t)) && readOK)
        *readOK = false;

    ret = ntohll(ret); // Reconvert into host based integer.
    return ret;
}


bool Stream_Reader::readBlock32(void* data, uint32_t datalen, bool keepDataLen)
{
    bool readOK;
    uint32_t len;
    if ((len = readU32(&readOK)) != 0 && readOK)
    {
        if (len != datalen && !keepDataLen)
            return false;
        uint32_t r;
        return (readBlock(data, len, &r) && r==len);
    }
    return false;
}

char * Stream_Reader::readBlock32WAlloc(uint32_t *datalen)
{
    bool readOK;
    uint32_t len;
    if ((len = readU32(&readOK)) != 0 && readOK)
    {
        if (*datalen < len)  // len received exceeded the max datalen permited.
        {
            *datalen = 0;
            return nullptr;
        }
        // download and resize
        char * odata = new char[len];
        if (!odata)
            return nullptr; // not enough memory.
        uint32_t r;
        bool ok = readBlock(odata, len,&r) && r==len;
        if (!ok)
        {
            delete [] odata;
            *datalen = 0;
            return nullptr;
        }
        *datalen = len;
        return odata;
    }
    *datalen = 0;
    return nullptr;
}

// TODO: null termination

int32_t Stream_Reader::read64KBlockDelim(char * block, const char *delim, const uint16_t &delimBytes, const uint32_t & blockNo)
{
    bool readOK;

    for (unsigned int pos=1; pos<=65536; pos++)
    {
        block[pos-1]=readU8(&readOK);
        if (!readOK) return -2; // FAILED WITH READ ERROR.

        if ((blockNo==0 && pos>=delimBytes) || blockNo>0)
        {
            char * comparisson_point = block+(pos-1)-(delimBytes-1);
            if (memcmp(comparisson_point, delim, delimBytes)==0)
                return static_cast<int32_t>(pos);
        }
    }
    return -1; // not found.
}

char* Stream_Reader::readBlock32WAllocAndDelim(unsigned int* datalen,
        const char *delim, uint16_t delimBytes)
{
    if (*datalen<=65535) return nullptr; // It should at least have 65k of buffer.

    char * currentBlock = new char[65536];
    unsigned int currentBlockSize = 65536;
    unsigned int blockNo = 0;

    while (true)
    {
        int bytesRecv = read64KBlockDelim(currentBlock+currentBlockSize-65536, delim, delimBytes, blockNo);
        if (bytesRecv == -2)
        {
            // maybe connection closed... returning nullptr;
            delete [] currentBlock;
            return nullptr;
        }
        else if (bytesRecv == -1)
        {
            if (currentBlockSize+65536>*datalen)
            {
                // Can't request more memory. erase current...
                delete [] currentBlock;
                return nullptr;
            }
            else
            {
                // Requesting more memory... and copying the old block into the new block.
                char * nextBlock = new char[currentBlockSize+65536];
                blockNo++;
                memcpy(nextBlock, currentBlock, currentBlockSize);
                delete [] currentBlock;
                currentBlock = nextBlock;
                currentBlockSize+=65536;
            }
        }
        else if (bytesRecv>0)
        {
            *datalen = currentBlockSize-65536+bytesRecv;
            return currentBlock;
        }
    }
}

bool Stream_Reader::readBlock16(void* data, uint16_t datalen,
        bool keepDataLen)
{
    bool readOK;
    uint16_t len;
    if ((len = readU16(&readOK)) != 0 && readOK)
    {
        if (len != datalen && !keepDataLen)
            return false;
        uint32_t r;
        return readBlock(data, len, &r) && r==len;
    }
    return false;
}

char *Stream_Reader::readBlockWAlloc(uint32_t *datalen, unsigned char sizel)
{
    if (!datalen) return nullptr;

    bool readOK = false;
    uint32_t lenReceived=0;

    if (sizel==8)
        lenReceived = (readU8(&readOK))+1;
    if (sizel==16)
        lenReceived = (readU16(&readOK))+1;
    if (sizel>16)
        lenReceived = (readU32(&readOK))+1;

    if (readOK)
    {
        if (lenReceived > *datalen)  // len received exceeded the max datalen permited.
        {
            *datalen = 0;
            return nullptr;
        }
        *datalen = lenReceived-1;
        // download and resize
        char * odata = new char[lenReceived];
        odata[lenReceived-1]=0;
        if (!odata) return nullptr; // not enough memory.
        uint32_t r;
        bool ok = readBlock(odata, lenReceived-1, &r) && r==lenReceived-1;
        if (!ok)
        {
            delete [] odata;
            *datalen = 0;
            return nullptr;
        }
        return odata;
    }
    else
    {
        *datalen = 0;
        return nullptr;
    }
}

bool Stream_Reader::readBlock8(void* data, uint8_t datalen,
        bool keepDataLen)
{
    bool readOK;
    uint8_t len;
    if ((len = readU8(&readOK)) != 0 && readOK)
    {
        if (len != datalen && !keepDataLen)
            return false;
        uint32_t r;
        return readBlock(data, len, &r) && r==len;
    }
    return false;
}

std::string Stream_Reader::readString(bool *readOK, unsigned char sizel)
{
    uint32_t receivedBytes = (1 << sizel)-1;

    if (readOK) *readOK = true;

    char * data = (char *)readBlockWAlloc(&receivedBytes,sizel);
    if (!data)
    {
        if (readOK) *readOK = false;
        return "";
    }

    if (!receivedBytes)
    {
        delete [] data;
        return "";
    }
    else
    {
        std::string v(data,receivedBytes);
        delete [] data;
        return v;
    }
}
