#ifndef BINARYCONTAINERCHUNK
#define BINARYCONTAINERCHUNK

#include <string.h>
#include <stdint.h>

#include "common.h"

struct BinaryContainerChunk {
    /**
     * @brief BinaryContainerChunk Constructor
     */
    BinaryContainerChunk()
    {
        rodata = nullptr;
        rosize = 0;
        data = nullptr;
        size = 0;
        offset = 0;
    }

    /**
     * @brief destroy the chunk data (by the inside)
     */
    void destroy()
    {
        if (data) delete [] data;
        data = nullptr;
        size = 0;
    }

    /**
     * @brief Displace Move data backwards, removing the first n bytes.
     * @param displLen number of bytes to be displaced
     */
    void displace(size_t displLen)
    {
        if (!data) return;
        if (!displLen) return;
        if (displLen>=size)
        {
            destroy();
            return;
        }

        size_t nContainerSize = size-displLen;
        char * ndata = new char[nContainerSize];
        memcpy(ndata,data+displLen,nContainerSize);
        destroy();
        data = ndata;
        size = nContainerSize;

        //** Remember to rearrange next offsets...
    }

    void truncate(uint64_t nSize)
    {
        if (nSize>=(offset+size)) return;
        if (!nSize || nSize == offset) return;

        // Current new chunk size.
        nSize-=offset;

        char * ndata = new char[nSize];
        memcpy(ndata,data,nSize);

        destroy();

        data = ndata;
        size = nSize;
    }

    /**
     * @brief Creates a new memory space and copy linear data inside.
     * @param buf pointer of data to be copied.
     * @param count size of data to be copied.
     * @return true if succeed. false otherwise.
     */
    bool copy(const void * buf, size_t count)
    {
        destroy();
        data = new char[count];
        if (!data) return false;
        size = count;
        memcpy(data,buf,count);
        return true;
    }

    /**
     * @brief Offset of the next/following chunk.
     * @return Current Offset + Size of this container (absolute offset in bytes of the next chunk)
     */
    uint64_t nextOffset()
    {
        return size+offset;
    }

    /**
     * @brief Ask if some absolute offset belongs to this chunk.
     * @param l_offset requested absolute offset in bytes.
     * @return true if the requested offset is on this chunk
     */
    bool containsOffset(const uint64_t &l_offset)
    {
        return (l_offset>=offset && l_offset<nextOffset());
    }

    /**
     * @brief Move the chunk to some specific offset for manipulation
     * COPY THE CHUNK REFERENCE FIRST. DON'T USE DIRECTLY ON "chunksVector"
     * @param l_offset offset to be displaced in bytes
     */
    void moveToOffset(uint64_t l_offset)
    {
        l_offset -= offset;
        offset = 0;
        size -= l_offset;
        data += l_offset;
    }

    const char * rodata;
    size_t rosize;

    char * data;
    size_t size;
    uint64_t offset;
};


#endif // BINARYCONTAINERCHUNK

