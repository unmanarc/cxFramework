#include "binarycontainer_chunks.h"
#include <string.h>

BinaryContainer_Chunks::BinaryContainer_Chunks()
{
    storeMethod = BC_METHOD_CHUNKS;

    mmapContainer = nullptr;
    maxChunkSize = 64*KB_MULT; // 64Kb.
    maxChunks = 256*KB_MULT; // 256K chunks (not more)
    maxContainerSizeUntilGoingToFS = 32*MB_MULT; // 32Mb.

    BinaryContainer_Chunks::clear2();
}

BinaryContainer_Chunks::~BinaryContainer_Chunks()
{
    BinaryContainer_Chunks::clear2();
}

void BinaryContainer_Chunks::setMaxContainerSizeUntilGoingToFS(const uint64_t &value)
{
    maxContainerSizeUntilGoingToFS = value;
}

uint64_t BinaryContainer_Chunks::getMaxContainerSizeUntilGoingToFS() const
{
    return maxContainerSizeUntilGoingToFS;
}

bool BinaryContainer_Chunks::isUsingFiles()
{
    return mmapContainer!=nullptr;
}

size_t BinaryContainer_Chunks::getMaxChunks() const
{
    return maxChunks;
}

void BinaryContainer_Chunks::setMaxChunks(const size_t &value)
{
    if (value<std::numeric_limits<uint32_t>::max())
    {
        maxChunks = value;
    }
    else
    {
        maxChunks = std::numeric_limits<uint32_t>::max();
    }
}

void BinaryContainer_Chunks::setMaxChunkSize(const uint32_t &value)
{
    maxChunkSize = value;
}

uint64_t BinaryContainer_Chunks::size() const
{
    if (mmapContainer) return mmapContainer->size();
    //std::cout << "BinaryContainer_Chunks::  Getting size() " << containerBytes << std::endl << std::flush;
    return containerBytes;
}

std::pair<bool, uint64_t> BinaryContainer_Chunks::truncate2(const uint64_t &bytes)
{
    if (mmapContainer)
    {
        return mmapContainer->truncate(bytes);
    }

    size_t ival = I_Chunk_GetPosForOffset(bytes);
    if (ival==MAX_SIZE_T)
        return std::make_pair(false,(uint64_t)0);

    chunksVector[ival].truncate(bytes);

    for (int i=static_cast<int>(ival);i<static_cast<int>(chunksVector.size());i++)
    {
        chunksVector[ival].destroy();
        chunksVector.erase(chunksVector.begin()+i);
    }

    return std::make_pair(true,size());
}

std::pair<bool, uint64_t> BinaryContainer_Chunks::append2(const void *buf, const uint64_t &roLen, bool prependMode)
{
    uint64_t len = roLen;
    if (mmapContainer && !prependMode) return mmapContainer->append(buf,len);
    if (mmapContainer && prependMode) return mmapContainer->prepend(buf,len);

    std::pair<bool,uint64_t> appendedBytes = std::make_pair(true,0);

    // Offset:bytes will overflow...
    if (CHECK_UINT_OVERFLOW_SUM(len,size()))
        return std::make_pair(false,(uint64_t)0);

    if (maxContainerSizeUntilGoingToFS!=0 && len+size() > maxContainerSizeUntilGoingToFS)
    {
        mmapContainer = copyToFS("",true);
        if (!mmapContainer)
            return std::make_pair(false,(uint64_t)0);
        clearChunks(); // Clear this container leaving the mmap intact...
        if (prependMode)
            return mmapContainer->prepend(buf,len);
        else
            return mmapContainer->append(buf,len);
    }

    while (len)
    {
        uint64_t chunkSize = len<maxChunkSize? len:maxChunkSize;

        // Don't create new chunks if we can't handle them.
        if (chunksVector.size()+1>maxChunks)
        {
            appendedBytes.first = false;
            return appendedBytes;
        }

        ///////////////////////////////////////////////////////
        // Copy memory:
        BinaryContainerChunk bcc;
        if (!bcc.copy(buf,chunkSize))
        {
            if (prependMode) recalcChunkOffsets();
            appendedBytes.first = false;
            return appendedBytes; // not enough memory.
        }

        ///////////////////////////////////////////////////////
        // Append or prepend the data.
        if (!prependMode)
        {
            if (!chunksVector.size()) bcc.offset = 0;
            else bcc.offset = chunksVector[chunksVector.size()-1].nextOffset();
            chunksVector.push_back(bcc);
        }
        else
        {
            chunksVector.emplace( chunksVector.begin(), bcc );
        }

        ///////////////////////////////////////////////////////
        // Update the size of the container
        incContainerBytesCount(bcc.size);
        appendedBytes.second+=bcc.size;

        ////////////////////////////
        // local counters update...
        buf=((const char *)buf)+chunkSize;
        len-=chunkSize;
    }

    if (prependMode) recalcChunkOffsets();
    return appendedBytes;
}

std::pair<bool,uint64_t> BinaryContainer_Chunks::displace2(const uint64_t &roBytesToDisplace)
{
    uint64_t bytesToDisplace = roBytesToDisplace;
    if (mmapContainer) return mmapContainer->displace(bytesToDisplace);

    std::pair<bool,uint64_t> displaced = std::make_pair(true,0);

    while (bytesToDisplace)
    {
        if (!chunksVector.size()) return displaced; // not completely displaced

        if (bytesToDisplace >= chunksVector[0].size)
        {
            // remove this chunk entirely
            displaced.second += chunksVector[0].size;
            bytesToDisplace-=chunksVector[0].size;
            decContainerBytesCount(chunksVector[0].size);
            chunksVector[0].destroy();
            chunksVector.erase(chunksVector.begin());
        }
        else
        {
            // displace the chunk partially.
            displaced.second += bytesToDisplace;
            chunksVector[0].displace(bytesToDisplace);
            decContainerBytesCount(bytesToDisplace);
            bytesToDisplace = 0;
        }
    }
    // Rearrange offsets here.
    recalcChunkOffsets();
    return displaced;
}

bool BinaryContainer_Chunks::clear2()
{
    return clearMmapedContainer() && clearChunks();
}

bool BinaryContainer_Chunks::clearMmapedContainer()
{
    if (mmapContainer) delete mmapContainer;
    mmapContainer = nullptr;
    return true;
}

bool BinaryContainer_Chunks::clearChunks()
{
    for (BinaryContainerChunk bcc : chunksVector)
        bcc.destroy();
    chunksVector.clear();
    return true;
}

std::pair<bool,uint64_t> BinaryContainer_Chunks::copyToStream2(std::ostream &bc, const uint64_t &roBytes, const uint64_t &roOffset)
{
    uint64_t bytes = roBytes;
    uint64_t offset = roOffset;
    if (mmapContainer) return mmapContainer->copyToStream(bc,bytes,offset);

    if (!bytes) return std::make_pair(true,0);

    // Offset:bytes will overflow...
    if (CHECK_UINT_OVERFLOW_SUM(offset,bytes)) return std::make_pair(false,(uint64_t)0);
    // No bytes to copy:
    if (offset>size()) return std::make_pair(false,(uint64_t)0);
    // Request exceed this container.
    if (offset+bytes>size()) bytes = size()-offset;

    uint64_t dataToCopy = bytes;
    std::vector<BinaryContainerChunk> copyChunks;

    // iterate over chunks and put that data on the new bc.
    for (auto & i : chunksVector)
    {
        BinaryContainerChunk currentChunk = i;

        // arrange from non-ro elements.
        if (currentChunk.rodata == nullptr)
        {
            currentChunk.rodata = currentChunk.data;
            currentChunk.rosize = currentChunk.size;
        }

        if (offset>0)
        {
            if (offset>currentChunk.rosize)
            {
                // pass this chunk...
                offset-=currentChunk.rosize;
                continue; // chunk consumed.
            }
            else
            {
                currentChunk.rosize-=offset;
                currentChunk.rodata+=offset;
                offset = 0;
            }
        }

        if (!offset)
        {
            currentChunk.rosize = dataToCopy>currentChunk.rosize?currentChunk.rosize:dataToCopy;
            copyChunks.push_back(currentChunk);
            dataToCopy-=currentChunk.rosize;
            if (!dataToCopy) break; // :)
        }
    }

    return std::make_pair(true,copyToStreamUsingCleanVector(bc,copyChunks));
}

std::pair<bool,uint64_t> BinaryContainer_Chunks::copyTo2(StreamableObject &bc, WRStatus & wrStatUpd, const uint64_t &roBytes, const uint64_t &roOffset)
{
    uint64_t bytes = roBytes;
    uint64_t offset = roOffset;
    if (mmapContainer) return mmapContainer->appendTo(bc,wrStatUpd,bytes,offset);

    if (!bytes) return std::make_pair(true,0);

    // Offset:bytes will overflow...
    if (CHECK_UINT_OVERFLOW_SUM(offset,bytes)) return std::make_pair(false,(uint64_t)0);
    // No bytes to copy:
    if (offset>size()) return std::make_pair(false,(uint64_t)0);
    // Request exceed this container.
    if (offset+bytes>size()) bytes = size()-offset;

    uint64_t dataToCopy = bytes;
    std::vector<BinaryContainerChunk> copyChunks;

    // iterate over chunks and put that data on the new bc.
    for (auto & i : chunksVector)
    {
        BinaryContainerChunk currentChunk = i;

        // arrange from non-ro elements.
        if (currentChunk.rodata == nullptr)
        {
            currentChunk.rodata = currentChunk.data;
            currentChunk.rosize = currentChunk.size;
        }

        if (offset>0)
        {
            if (offset>currentChunk.rosize)
            {
                // pass this chunk...
                offset-=currentChunk.rosize;
                continue; // chunk consumed.
            }
            else
            {
                currentChunk.rosize-=offset;
                currentChunk.rodata+=offset;
                offset = 0;
            }
        }

        if (!offset)
        {
            currentChunk.rosize = dataToCopy>currentChunk.rosize?currentChunk.rosize:dataToCopy;
            copyChunks.push_back(currentChunk);
            dataToCopy-=currentChunk.rosize;
            if (!dataToCopy) break; // :)
        }
    }

    return std::make_pair(true,copyToSOUsingCleanVector(bc,copyChunks,wrStatUpd));
}

std::pair<bool, uint64_t> BinaryContainer_Chunks::copyOut2(void *buf, const uint64_t &roBytes, const uint64_t &offset)
{
    uint64_t bytes = roBytes;
    if (mmapContainer) return mmapContainer->copyOut(buf,bytes,offset);

    uint64_t copiedBytes = 0;

    // Offset:bytes will overflow...
    if (CHECK_UINT_OVERFLOW_SUM(offset,bytes)) return std::make_pair(false,(uint64_t)0);

    // No bytes to copy:
    if (!bytes) return std::make_pair(true,0);

    // out of bounds (fail to copy):
    if (offset+bytes>size()) return std::make_pair(false,(uint64_t)0);

    ////////////////////////////////////

    size_t icurrentChunk = I_Chunk_GetPosForOffset(offset);
    if (icurrentChunk==MAX_SIZE_T) return std::make_pair(false,(uint64_t)0);

    BinaryContainerChunk currentChunk = chunksVector[icurrentChunk];
    currentChunk.moveToOffset(offset);

    while (bytes)
    {
        if (bytes > currentChunk.size)
        {
            // copy the whole chunk.
            memcpy(buf,currentChunk.data,currentChunk.size);

            // repos the vars...
            copiedBytes += currentChunk.size;
            bytes-=currentChunk.size;
            buf=((char *)buf)+currentChunk.size;
        }
        else if (bytes <= currentChunk.size)
        {
            // Copy part of the chunk.
            memcpy(buf,currentChunk.data,bytes);

            // ends here.
            return std::make_pair(true,copiedBytes+bytes);
        }

        // proceed to the next chunk...
        if (icurrentChunk==chunksVector.size()-1) break;
        icurrentChunk++;
        currentChunk = chunksVector[icurrentChunk];
    }

    return std::make_pair(true,copiedBytes);
}

bool BinaryContainer_Chunks::compare2(const void *buf, const uint64_t &len, bool caseSensitive, const uint64_t &roOffset)
{
    uint64_t offset = roOffset;
    if (mmapContainer) return mmapContainer->compare(buf,len,caseSensitive,offset);

    // Offset:bytes will overflow...
    if (CHECK_UINT_OVERFLOW_SUM(offset,len)) return false;

    // No bytes to copy:
    if (!len) return true;

    // out of bounds (fail to compare):
    if (offset+len>size()) return false;

    /////////////////////////////
    uint64_t dataToCompare = len, dataCompared = 0;

    // iterate over chunks and put that data on the new bc.

    //for (auto & i : chunksVector)
    size_t vpos=0, vsize = chunksVector.size();
    for (  ; vpos<vsize ; vpos++ )
    {
        BinaryContainerChunk * i = &(chunksVector[vpos]);
        BinaryContainerChunk currentChunk = *i; // copy the chunk..

        // TODO: CHECK.
        // if offset is >0...
        if (offset>0)
        {
            if (offset>i->size)
            {
                // pass this chunk...
                offset-=i->size;
                continue; // chunk consumed.
            }
            else
            {
                currentChunk.size-=offset;
                currentChunk.data+=offset;

                offset = 0;
            }
        }

        if (!offset)
        {
            size_t currentChunkSize = dataToCompare>currentChunk.size?currentChunk.size:dataToCompare;

            if (memicmp2(currentChunk.data, buf,currentChunkSize,caseSensitive)) return false; // does not match!

            dataToCompare-=currentChunkSize;
            dataCompared+=currentChunkSize;
            buf=((const char *)buf)+currentChunkSize;

            // Ended.!
            if (!dataToCompare) return true;
        }
    }

    // TODO: check the logic here
    // If there is any data to compare left, return false.
    return dataToCompare==0;
}




// TODO: ICASE
std::pair<bool, uint64_t> BinaryContainer_Chunks::findChar(const int &c, const uint64_t &roOffset, uint64_t searchSpace, bool caseSensitive)
{
    if (caseSensitive && !(c>='A' && c<='Z') && !(c>='a' && c<='z') )
        caseSensitive = false;

    uint64_t offset = roOffset;
    if (mmapContainer) return mmapContainer->findChar(c,offset);

    ///////////////////////////
    size_t currentSize = size();
    if (CHECK_UINT_OVERFLOW_SUM(offset,searchSpace)) return std::make_pair(false,std::numeric_limits<uint64_t>::max());
    // out of bounds (fail to compare):
    if (offset>currentSize || offset+searchSpace>currentSize) return std::make_pair(false,std::numeric_limits<uint64_t>::max());

    size_t retpos = 0;
    size_t vpos=0, vsize = chunksVector.size();
    for (  ; vpos<vsize ; vpos++ )
    {
        BinaryContainerChunk * originalChunk = &(chunksVector[vpos]);
        BinaryContainerChunk currentChunk = *originalChunk; // copy the chunk..

        // if offset is >0...
        if (offset>0)
        {
            if (offset>originalChunk->size)
            {
                // pass this chunk...
                offset-=originalChunk->size;
                // chunk discarded.
                retpos+=originalChunk->size;

                continue; // chunk consumed.
            }
            else
            {
                currentChunk.size-=offset;
                currentChunk.data+=offset;

                offset = 0;
            }
        }

        if (offset==0)
        {
            char * pos = nullptr;

            if (!caseSensitive)
                pos = (char *)memchr(currentChunk.data, c, searchSpace>currentChunk.size? currentChunk.size : searchSpace);
            else
            {
                char *pos_upper = (char *)memchr(currentChunk.data, std::toupper(c), searchSpace>currentChunk.size? currentChunk.size : searchSpace);
                char *pos_lower = (char *)memchr(currentChunk.data, std::tolower(c), searchSpace>currentChunk.size? currentChunk.size : searchSpace);

                if      (pos_upper && pos_lower && pos_upper<=pos_lower) pos = pos_upper;
                else if (pos_upper && pos_lower && pos_lower<pos_upper) pos = pos_lower;
                else if (pos_upper) pos = pos_upper;
                pos = pos_lower;
            }

            if (pos)
            {
                // report the position.
                const ptrdiff_t bytepos = pos-(originalChunk->data);
                return std::make_pair(true,bytepos+retpos);
            }

            if (searchSpace>currentChunk.size)
                searchSpace-=currentChunk.size;
            else
                 return std::make_pair(false,(uint64_t)0);
        }

        // chunk discarded.
        retpos+=originalChunk->size;
    }
    return std::make_pair(false,(uint64_t)0);
}

void BinaryContainer_Chunks::recalcChunkOffsets()
{
    unsigned long long currentOffset = 0;
    size_t vpos=0, vsize = chunksVector.size();
    for ( BinaryContainerChunk * i = &(chunksVector[vpos]) ; vpos<vsize ; vpos++ )
    {
        i->offset = currentOffset;
        currentOffset = i->nextOffset();
    }
}

size_t BinaryContainer_Chunks::I_Chunk_GetPosForOffset(const uint64_t &offset,  size_t  curpos, size_t curmax, size_t curmin)
{
    // The Search Algorithm!
    if (!chunksVector.size()) return MAX_SIZE_T;

    // Boundaries definition:
    if (curpos == MAX_SIZE_T)
        curpos = chunksVector.size()==1?0:(chunksVector.size()/2)-1;
    if (curmax == MAX_SIZE_T)
        curmax = chunksVector.size()==1?0:chunksVector.size()-1;
    if (curmin == MAX_SIZE_T)
        curmin = 0;

    ////////////////////////////////////////////////////////////////
    // Verify current chunk. (VERIFICATION)
    if (chunksVector[curpos].containsOffset(offset))
    {
        return curpos;
    }
    ////////////////////////////////////////////////////////////////

    // If not found, continue searching:
    if ( offset < chunksVector[curpos].offset )
    {
        // search down. (from curmin to curpos-1)
        if (curpos == curmin) return MAX_SIZE_T; // Not any element down.
        ///////////////
        curmax = curpos-1;
        return I_Chunk_GetPosForOffset(offset, curmin==curmax? curmin : curmin+((curmax+1-curmin)/2)-(curmax-curmin)%2, curmax, curmin );
    }
    else
    {
        // search up. (from curpos+1 to curmax)
        if (curpos == curmax) return MAX_SIZE_T; // Not any element up.
        curmin = curpos+1;
        return I_Chunk_GetPosForOffset(offset, curmin==curmax? curmin : curmin+((curmax+1-curmin)/2)-(curmax-curmin)%2, curmax, curmin );
    }
}

BinaryContainer_MMAP *BinaryContainer_Chunks::getMmapContainer() const
{
    return ((BinaryContainer_MMAP *)mmapContainer);
}

