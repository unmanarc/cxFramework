#include "binarycontainer_mem.h"

BinaryContainer_MEM::BinaryContainer_MEM(const void *buf, const uint32_t & len)
{
    storeMethod = BC_METHOD_MEM;
    linearMem = nullptr;
    readOnly = true;
    setContainerBytes(0);
    BinaryContainer_MEM::clear2();
    if (buf && len) reference(buf,len);
}

BinaryContainer_MEM::~BinaryContainer_MEM()
{
    BinaryContainer_MEM::clear2();
}

void BinaryContainer_MEM::reference(const void *buf, const uint32_t &len)
{
    clear();
    linearMem = ((const char *)buf);
    readOnly = true;
    setContainerBytes(len);
}

std::pair<bool, uint64_t> BinaryContainer_MEM::findChar(const int &c, const uint64_t &offset, uint64_t searchSpace, bool caseSensitive)
{
    if (caseSensitive && !(c>='A' && c<='Z') && !(c>='a' && c<='z') )
        caseSensitive = false;

    size_t currentSize = size();
    // No bytes to copy.
    if (!currentSize) return std::make_pair(false,(uint64_t)0);

    if (CHECK_UINT_OVERFLOW_SUM(offset,searchSpace)) return std::make_pair(false,std::numeric_limits<uint64_t>::max());

    // No bytes to copy:
    if (offset+searchSpace>currentSize) return  std::make_pair(false,std::numeric_limits<uint64_t>::max());

    if (searchSpace == 0) searchSpace = currentSize-offset;

    const char * cPos = nullptr;//

    if (!caseSensitive)
        cPos = (const char *)memchr(linearMem+offset,c,searchSpace);
    else
    {
        const char *pos_upper = (const char *)memchr(linearMem+offset,std::toupper(c),searchSpace);
        const char *pos_lower = (const char *)memchr(linearMem+offset,std::tolower(c),searchSpace);

        if      (pos_upper && pos_lower && pos_upper<=pos_lower) cPos = pos_upper;
        else if (pos_upper && pos_lower && pos_lower<pos_upper) cPos = pos_lower;
        else if (pos_upper) cPos = pos_upper;
        cPos = pos_lower;
    }

    if (!cPos) return std::make_pair(false,(uint64_t)0);

    const ptrdiff_t bytepos = cPos-linearMem;
    return std::make_pair(true,bytepos);
}

std::pair<bool, uint64_t> BinaryContainer_MEM::truncate2(const uint64_t &bytes)
{
    setContainerBytes(bytes);
    return std::make_pair(true,size());
}

std::pair<bool, uint64_t> BinaryContainer_MEM::append2(const void *, const uint64_t &, bool )
{
    return std::make_pair(false,(uint64_t)0);
}

std::pair<bool, uint64_t> BinaryContainer_MEM::displace2(const uint64_t &bytesToDisplace)
{
    if (bytesToDisplace>size()) return std::make_pair(false,(uint64_t)0);

    linearMem+=bytesToDisplace;
    decContainerBytesCount(bytesToDisplace);

    return std::make_pair(true,bytesToDisplace);
}

bool BinaryContainer_MEM::clear2()
{
    linearMem = nullptr;
    setContainerBytes(0);
    return true;
}

std::pair<bool,uint64_t> BinaryContainer_MEM::copyToStream2(std::ostream &bc, const uint64_t &roBytes, const uint64_t &offset)
{
    size_t currentBytes = size();
    uint64_t bytes = roBytes;
    // No bytes to copy.
    if (!bytes) return std::make_pair(true,0);

    // Offset:bytes will overflow...
    if (CHECK_UINT_OVERFLOW_SUM(offset,bytes)) return std::make_pair(false,(uint64_t)0);
    // No bytes to copy:
    if (offset>currentBytes) return std::make_pair(false,(uint64_t)0);
    // Request exceed this container, bytes should only copy what's right...
    if (offset+bytes>currentBytes) bytes = currentBytes-offset;

    uint64_t dataToCopy = bytes;
    std::vector<BinaryContainerChunk> copyChunks;

    const char * transmitMem = linearMem+offset;

    while (dataToCopy)
    {
        BinaryContainerChunk bcx;

        bcx.rosize = dataToCopy>64*KB_MULT?64*KB_MULT:dataToCopy;
        bcx.rodata = transmitMem;

        copyChunks.push_back(bcx);

        transmitMem+=bcx.rosize;
        dataToCopy-=bcx.rosize;
    }

    return std::make_pair(true,copyToStreamUsingCleanVector(bc,copyChunks));
}

std::pair<bool,uint64_t> BinaryContainer_MEM::copyTo2(StreamableObject &bc, WRStatus & wrStatUpd, const uint64_t &roBytes, const uint64_t &offset)
{
    uint64_t bytes = roBytes;
    // No bytes to copy.
    if (!bytes) return std::make_pair(true,0);

    // Offset:bytes will overflow...
    if (CHECK_UINT_OVERFLOW_SUM(offset,bytes)) return std::make_pair(false,(uint64_t)0);
    // No bytes to copy:
    if (offset>size()) return std::make_pair(false,(uint64_t)0);
    // Request exceed this container, bytes should only copy what's right...
    if (offset+bytes>size()) bytes = size()-offset;

    uint64_t dataToCopy = bytes;
    std::vector<BinaryContainerChunk> copyChunks;

    const char * transmitMem = linearMem+offset;

    while (dataToCopy)
    {
        BinaryContainerChunk bcx;

        bcx.rosize = dataToCopy>64*KB_MULT?64*KB_MULT:dataToCopy;
        bcx.rodata = transmitMem;

        copyChunks.push_back(bcx);

        transmitMem+=bcx.rosize;
        dataToCopy-=bcx.rosize;
    }

    return  std::make_pair(true,copyToSOUsingCleanVector(bc,copyChunks,wrStatUpd));
}

std::pair<bool,uint64_t> BinaryContainer_MEM::copyOut2(void *buf, const uint64_t &bytes, const uint64_t &offset)
{
    // Offset:bytes will overflow...
    if (CHECK_UINT_OVERFLOW_SUM(offset,bytes)) return std::make_pair(false,(uint64_t)0);

    // No bytes to copy:
    if (!bytes) return std::make_pair(true,0);

    // out of bounds (fail to copy):
    if (offset+bytes>size()) return std::make_pair(false,(uint64_t)0);

    ////////////////////////////////////

    // error: offset exceed the container size (try another offset)
    if (offset>size()) return std::make_pair(false,(uint64_t)0);

    const char * linearMemOffseted = linearMem+offset;
    uint64_t containerBytesOffseted = size()-offset;
    if (containerBytesOffseted==0)
        return std::make_pair(true,0); // no data left to copy. (copy 0 bytes)
    uint64_t copiedBytes = containerBytesOffseted<bytes?containerBytesOffseted:bytes;

    memcpy(buf,linearMemOffseted,copiedBytes);

    return std::make_pair(false,copiedBytes);
}

bool BinaryContainer_MEM::compare2(const void *buf, const uint64_t &len, bool caseSensitive, const uint64_t &offset)
{
    // Offset:bytes will overflow...
    if (CHECK_UINT_OVERFLOW_SUM(offset,len)) return false;

    // No bytes to copy:
    if (!len) return true;

    // out of bounds (fail to compare):
    if (offset+len>size()) return false;

    /////////////////////////////

    const char * memoryToBeCompared = linearMem + offset;
    return !memicmp2(memoryToBeCompared, buf, len,caseSensitive);
}

