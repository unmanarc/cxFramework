#include "binarycontainer_base.h"

#include "binarycontainer_mmap.h"
#include "binarycontainer_ref.h"

#include <limits>

BinaryContainer_Base::BinaryContainer_Base()
{
    maxSize = std::numeric_limits<uint64_t>::max();
    readOnly = false;
    storeMethod = BC_METHOD_NULL;
    clear0();
}

BinaryContainer_Base::~BinaryContainer_Base()
{
    clear0();
}

BinaryContainer_Base &BinaryContainer_Base::operator=(BinaryContainer_Base &bc)
{
    clear();
    bc.appendTo(*this);
    return *this;
}

void BinaryContainer_Base::print(FILE *f)
{
    fprintf(f, "%s", toString().c_str());
}

std::pair<bool,uint64_t> BinaryContainer_Base::prepend(const void *buf)
{
    return prepend(buf,strlen((const char *)buf));
}

std::pair<bool,uint64_t> BinaryContainer_Base::append(const void *buf)
{
    return append(buf,strlen((const char *)buf));
}

std::pair<bool,uint64_t> BinaryContainer_Base::append(const void *data, uint64_t len)
{
    // Read only: can't append nothing.
    if (readOnly) return std::make_pair(false,(uint64_t)0);

    uint64_t currentSize = size();

    // New size will overflow the counter...
    if (CHECK_UINT_OVERFLOW_SUM(len,currentSize))
        return std::make_pair(false,(uint64_t)0);
    // out of bounds, fail.
    if (len+currentSize>maxSize)
        return std::make_pair(false,(uint64_t)0);
    // zero to copy!
    if (!len)
        return std::make_pair(true,0);

    // Data modification should pass trough referenced pointer.
    return append2(data,len,false);
}

std::pair<bool,uint64_t> BinaryContainer_Base::prepend(const void *data, uint64_t len)
{
    // Read only: can't append nothing.
    if (readOnly) return std::make_pair(false,(uint64_t)0);

    uint64_t currentSize = size();

    // New size will overflow the counter...
    if (CHECK_UINT_OVERFLOW_SUM(len,currentSize))
        return std::make_pair(false,(uint64_t)0);
    // out of bounds.
    if (len+currentSize>maxSize)
        return std::make_pair(false,(uint64_t)0);
    // zero to copy!
    if (!len)
        return std::make_pair(true,0);

    return append2(data,len,true);
}

std::pair<bool, uint64_t> BinaryContainer_Base::displace(const uint64_t &bytes)
{
    uint64_t currentSize = size();
    return displace2(bytes>currentSize?currentSize:bytes);
}

std::pair<bool,uint64_t> BinaryContainer_Base::truncate(const uint64_t &bytes)
{
    uint64_t currentSize = size();

    if (bytes>=currentSize)
        return std::make_pair(false,currentSize); //not truncated.

    return truncate2(bytes);
}

bool BinaryContainer_Base::clear0()
{
    containerBytes = 0;
    fsDirectoryPath = "/tmp";
    fsBaseFileName = "BinaryContainer-";
    return true;
}

bool BinaryContainer_Base::clear()
{
    clear0();
    return clear2();
}

int BinaryContainer_Base::copyUntil(BinaryContainer_Base &destination, const void *needle, const size_t &needleLenght, const uint64_t &maxCopySize, bool removeNeedle)
{
    std::pair<bool,uint64_t> needlePos = find(needle,needleLenght,false,0,maxCopySize);

    if (!needlePos.first)
        return -1;

    // will overflow...
    if (CHECK_UINT_OVERFLOW_SUM(needlePos.second,needleLenght))
        return -1;

    if (removeNeedle)
    {
        if ((needlePos.second) > maxCopySize)
            return -2;
        appendTo(destination,needlePos.second);
    }
    else
    {
        if ((needlePos.second+needleLenght) > maxCopySize)
            return -2;
        appendTo(destination,needlePos.second+needleLenght);
    }

    return 0;
}

int BinaryContainer_Base::displaceUntil(BinaryContainer_Base &destination, const void *needle, const size_t &needleCount, const uint64_t &maxCopySize, bool removeNeedle)
{
    int retr = copyUntil(destination,needle,needleCount, maxCopySize, removeNeedle);
    if (retr<0) return retr;
    displace( destination.size() + (removeNeedle?needleCount:0) );
    return 0;
}

int BinaryContainer_Base::displaceUntil(BinaryContainer_Base &destination, const std::list<std::string> needles, const uint64_t &maxCopySize, bool removeNeedle)
{
    for (const auto & needle : needles)
    {
        if (!displaceUntil(destination,needle.c_str(), needle.size(), maxCopySize, removeNeedle))
            return 0;
    }
    return -1;
}

// TODO: pass this to shared_ptr
/*
std::list<BinaryContainer_Base *> BinaryContainer_Base::referencedSplit(const std::list<std::string> &needles,const uint64_t &maxSearchSize, const size_t &maxNeedles)
{
    std::list<std::string> skipBegWith;
    return referencedSplit2(needles,skipBegWith,maxSearchSize,maxNeedles);
}

std::list<BinaryContainer_Base *> BinaryContainer_Base::referencedSplit2(const std::list<std::string> &needles, const std::list<std::string> &skipBegWith, const uint64_t &maxSearchSize, const size_t &roMaxNeedles)
{
    size_t maxNeedles = roMaxNeedles;
    std::list<BinaryContainer_Base *> x;

    uint64_t currentOffset = 0;

    if (isNull()) return x;
    if (maxNeedles == 0) maxNeedles = std::numeric_limits<uint64_t>::max();

    BinaryContainer_Ref * current = new BinaryContainer_Ref;
    current->reference(this,currentOffset); // reference the whole container.
    maxNeedles--;

    if (maxNeedles == 0)
    {
        x.push_back(current);
        return x;
    }

    uint64_t currentLocalOffset = 0;

    for (size_t i=0;i<maxNeedles;i++)
    {
        // TODO: repair maxSearchSize.
        std::string needleFound;
        std::pair<bool,uint64_t> pos = current->find(needles,needleFound,currentLocalOffset,maxSearchSize);
        if (pos.first == false)
        {
            // needle not found... so this is the last element.
            x.push_back(current);
            return x;
        }
        else
        {
            // Displace the offset.
            currentOffset+=pos.second+needleFound.size();
            currentLocalOffset+=pos.second+needleFound.size();

            // Don't have more chunks. Get out.
            if (currentOffset>=size())
            {
                // Last element with another one at the end.
                current->truncate(current->size()-needleFound.size());
                x.push_back(current);
                x.push_back(nullptr);
                return x;
            }

            //////////////////////////////////////////////////////
            // Creates the next binary container.
            BinaryContainer_Ref * next = new BinaryContainer_Ref;
            next->reference(this,currentOffset);

            bool skipThis = false;
            for (std::string skip : skipBegWith)
            {
                if (next->compare(skip.c_str(),skip.size())) skipThis = true;
            }

            if (skipThis)
            {
                delete next;
                // skip to the next...
            }
            else
            {
                current->truncate(currentLocalOffset);
                x.push_back(current);
                current = next;
            }
        }
    }

    return x;
}*/

void BinaryContainer_Base::freeSplitList(std::list<BinaryContainer_Base *> x)
{
    for (auto i: x)
    {
        if (i) delete i;
    }
}

std::pair<bool,uint64_t> BinaryContainer_Base::copyToStream(std::ostream &out, uint64_t bytes, const uint64_t &offset)
{
    uint64_t currentSize = size();

    if (bytes == std::numeric_limits<uint64_t>::max())
    {
        if (offset>currentSize) return std::make_pair(false,(uint64_t)0); // invalid size.
        bytes = currentSize-offset; // whole container bytes copied.
    }

    // Offset:bytes will overflow...
    if (CHECK_UINT_OVERFLOW_SUM(offset,bytes)) return std::make_pair(false,(uint64_t)0);

    // No bytes to copy:
    if (!bytes) return std::make_pair(true,0);

    // out of bounds for sure.
    if (offset>currentSize) return std::make_pair(false,(uint64_t)0);

    // out of bounds (last bytes):
    if (offset+bytes>currentSize) bytes = currentSize-offset;

    ////////////////////////////////////
    return copyToStream2(out,bytes,offset);
}

std::pair<bool,uint64_t> BinaryContainer_Base::appendTo(StreamableObject &out, const uint64_t &bytes, const uint64_t &offset)
{
    WRStatus wrStat;
    return appendTo(out,wrStat,bytes,offset);
}

std::pair<bool,uint64_t> BinaryContainer_Base::appendTo(StreamableObject &out, WRStatus &wrStatUpd, uint64_t bytes, const uint64_t &offset)
{
    uint64_t currentSize = size();

    // Copy eveything...
    if (bytes == std::numeric_limits<uint64_t>::max())
    {
        if (offset>currentSize)
        {
            wrStatUpd.succeed=false;
            return std::make_pair(false,(uint64_t)0); // invalid pos.
        }
        bytes = currentSize-offset; // whole container bytes copied.
    }

    // Offset:bytes will overflow...
    if (CHECK_UINT_OVERFLOW_SUM(offset,bytes))
    {
        wrStatUpd.succeed=false;
        return std::make_pair(false,(uint64_t)0); // invalid pos.
    }

    // No bytes to copy:
    if (!bytes)
        return std::make_pair(true,0); // invalid pos.

    // out of bounds for sure.
    if (offset>currentSize)
    {
        wrStatUpd.succeed=false;
        return std::make_pair(false,(uint64_t)0); // invalid pos.
    }

    // out of bounds (last bytes):
    if (offset+bytes>currentSize) bytes = currentSize-offset;

    ////////////////////////////////////
    return copyTo2(out,wrStatUpd,bytes,offset);
}

std::pair<bool,uint64_t> BinaryContainer_Base::copyOut(void *buf, uint64_t bytes, const uint64_t &offset)
{
    uint64_t currentSize = size();

    if (bytes == std::numeric_limits<uint64_t>::max())
    {
        if (offset>currentSize) return std::make_pair(false,(uint64_t)0); // out of bounds.
        bytes = currentSize-offset; // whole container bytes copied.
    }

    // Offset:bytes will overflow...
    if (CHECK_UINT_OVERFLOW_SUM(offset,bytes)) return std::make_pair(false,(uint64_t)0);

    // No bytes to copy:
    if (!bytes) return std::make_pair(true,0);

    // out of bounds (fail to copy):
    if (offset+bytes>currentSize) return std::make_pair(false,(uint64_t)0);

    ////////////////////////////////////
    return copyOut2(buf,bytes,offset);
}

std::pair<bool, uint64_t> BinaryContainer_Base::copyToString(std::string &outStr, uint64_t bytes, const uint64_t &roOffset)
{
    uint64_t currentSize = size();

    uint64_t offset = roOffset;
    if (bytes == std::numeric_limits<uint64_t>::max())
    {
        if (offset>currentSize) return std::make_pair(false,(uint64_t)0); // out of bounds
        bytes = currentSize-offset; // whole container bytes copied.
    }

    // Offset:bytes will overflow...
    if (CHECK_UINT_OVERFLOW_SUM(offset,bytes)) return std::make_pair(false,(uint64_t)0);

    // No bytes to copy:
    if (!bytes) return std::make_pair(true,0);

    // out of bounds (fail to copy):
    if (offset+bytes>currentSize) return std::make_pair(false,(uint64_t)0);

    char outmem[8192];
    ////////////////////////////////////
    while (bytes)
    {
        size_t copyBytes = bytes>8192?8192:bytes;
        bytes-=copyBytes;

        std::pair<bool,uint64_t> outBytes = copyOut(outmem,copyBytes,offset);

        if (outBytes.first)
            outStr.append(outmem,outBytes.second);
        else
            return std::make_pair(false,outStr.size());

        offset+=copyBytes;
    }

    return std::make_pair(true,outStr.size());
}

std::string BinaryContainer_Base::toString(uint64_t bytes, const uint64_t &roOffset)
{
    std::string r;

    uint64_t currentSize = size();
    uint64_t offset = roOffset;

    if (bytes == std::numeric_limits<uint64_t>::max())
    {
        if (offset>currentSize)
            return r; // negative bytes copied.
        bytes = currentSize-offset; // whole container bytes copied.
    }

    // Offset:bytes will overflow...
    if (CHECK_UINT_OVERFLOW_SUM(offset,bytes))
        return r;

    // No bytes to copy:
    if (!bytes)
        return r;

    // out of bounds (fail to copy):
    if (offset+bytes>currentSize)
        return r;

    char outmem[8192];
    ////////////////////////////////////
    while (bytes)
    {
        size_t copyBytes = bytes>8192?8192:bytes;
        bytes-=copyBytes;
         std::pair<bool,uint64_t> outBytes = copyOut(outmem,copyBytes,offset);

         if (outBytes.first)
             r.append(outmem,outBytes.second);
         else
             return r;

        offset+=copyBytes;
    }

    return r;
}

uint64_t BinaryContainer_Base::toUInt64(int base, const uint64_t &bytes, const uint64_t &offset)
{
    return strtoull(toString(bytes,offset).c_str(),nullptr,base);
}

uint32_t BinaryContainer_Base::toUInt32(int base, const uint64_t &bytes, const uint64_t &offset)
{
    return strtoul(toString(bytes,offset).c_str(),nullptr,base);
}

bool BinaryContainer_Base::compare(const std::string &cmpString, bool caseSensitive, const uint64_t &offset)
{
    if (cmpString.size()!=size()) return false;
    return compare(cmpString.c_str(),cmpString.size(),caseSensitive,offset);
}

bool BinaryContainer_Base::compare(const void *mem, const uint64_t &len, bool caseSensitive, const uint64_t &offset)
{
    // Offset:bytes will overflow...
    if (CHECK_UINT_OVERFLOW_SUM(offset,len)) return false;

    // No bytes to copy:
    //if (!len || offset+len==size()) return true;
    // No data to compare...
    if (!len) return true;

    // out of bounds (fail to compare):
    if (offset+len>size()) return false;

    /////////////////////////////

    return compare2(mem,len,caseSensitive,offset);
}

std::pair<bool,uint64_t> BinaryContainer_Base::find(const void *needle,const size_t &needle_len, bool caseSensitive, const uint64_t &offset, uint64_t searchSpace)
{
    uint64_t currentSize = size();

    char * c_needle = (char *)needle;

    // Offset:bytes will overflow...
    if (CHECK_UINT_OVERFLOW_SUM(offset,needle_len)) return std::make_pair(false,std::numeric_limits<uint64_t>::max());

    // Offset:bytes will overflow...
    if (CHECK_UINT_OVERFLOW_SUM(offset,searchSpace)) return std::make_pair(false,std::numeric_limits<uint64_t>::max());

    if (offset>currentSize) return std::make_pair(false,std::numeric_limits<uint64_t>::max());
    if (searchSpace == 0) searchSpace = currentSize-offset;
    if (searchSpace == 0) return std::make_pair(false,(uint64_t)0);

    // not enough search space to found anything...
    if (searchSpace<needle_len) return std::make_pair(false,std::numeric_limits<uint64_t>::max());

    // nothing to be found... first position
    if (needle_len == 0) return std::make_pair(true,0);

    uint64_t currentOffset = offset;

    std::pair<bool,uint64_t> pos = findChar(c_needle[0],currentOffset,searchSpace,caseSensitive);
    while (pos.first == true) // char detected...
    {
        /////////////////////////////////
        uint64_t displacement = pos.second-currentOffset;

        currentOffset+=displacement;
        searchSpace-=displacement;

        if (compare2(needle,needle_len,caseSensitive,currentOffset))
            return std::make_pair(true,currentOffset);

        // no left space to consume.
        if (searchSpace == 0) break;

        // skip char found.
        currentOffset+=1;
        searchSpace-=1;

        pos = findChar(c_needle[0],currentOffset,searchSpace,caseSensitive);
    }

    // not found
    return std::make_pair(false,(uint64_t)0);

    /*
    if (searchSpace == 0) searchSpace = currentSize;

    //std::cout << "BinaryContainer_Base::find  Getting size() " << containerBytes << std::endl << std::flush;

    // Offset:bytes will overflow...
    if (CHECK_UINT_OVERFLOW_SUM(offset,needle_len)) return std::numeric_limits<uint64_t>::max();

    // Offset:bytes will overflow...
    if (CHECK_UINT_OVERFLOW_SUM(offset,searchSpace)) return std::numeric_limits<uint64_t>::max();

    // not enough search space to found anything...
    if (searchSpace<needle_len) return std::numeric_limits<uint64_t>::max();

    // nothing to found... first position
    if (needle_len == 0) return 0;

    // Reduce the needle size from the search space (optimization)
    searchSpace=searchSpace-needle_len+1;

    // calc search space available
    uint64_t availableSearchSpace= (currentSize-needle_len+1);

    // Verify if offset is appliable (bad offset)
    if (availableSearchSpace<offset) return std::numeric_limits<uint64_t>::max();

    // Apply offset:
    availableSearchSpace-=offset;

    // If searchSpace is lesser than available bytes... use searchSpace size instead.
    if (searchSpace<availableSearchSpace) availableSearchSpace=searchSpace;

    // start comparing at offset...
    for (uint64_t currentOffset=offset; currentOffset<availableSearchSpace; currentOffset++)
    {
        if (CHECK_UINT_OVERFLOW_SUM(currentOffset,needle_len)) std::numeric_limits<uint64_t>::max(); // ERROR...
        if (compare2(needle,needle_len,caseSensitive,currentOffset)) return currentOffset;
    }

    // TODO: found boolean flag instead.

    return std::numeric_limits<uint64_t>::max(); // not found.*/
}

std::pair<bool,uint64_t> BinaryContainer_Base::find(const std::list<std::string> &needles, std::string &needleFound, bool caseSensitive, const uint64_t &offset, const uint64_t &searchSpace)
{
    needleFound = "";
    for (const std::string & needle : needles)
    {
        std::pair<bool,uint64_t> f;
        if ((f=find(needle.c_str(),needle.size(),caseSensitive,offset,searchSpace)).first==true)
        {
            needleFound = needle;
            return f;
        }
    }
    return std::make_pair(false,(uint64_t)0);
}

uint64_t BinaryContainer_Base::size() const
{
    return containerBytes;
}

bool BinaryContainer_Base::isNull()
{
    return size()==0;
}

uint64_t BinaryContainer_Base::getMaxSize() const
{
    return maxSize;
}

void BinaryContainer_Base::setMaxSize(const uint64_t &value)
{
    maxSize = value;
}

bool BinaryContainer_Base::streamTo(StreamableObject *out, WRStatus &wrStatUpd)
{
    std::pair<bool,uint64_t> bytesAppended = appendTo(*out,wrStatUpd);
    if (bytesAppended.first==false)
    {
        out->writeEOF(false);
        return false;
    }
    else
    {
        out->writeEOF(true);
        return true;
    }
}

WRStatus BinaryContainer_Base::write(const void * buf, const size_t &count, WRStatus & wrStatUpd)
{
    WRStatus ret;
    std::pair<bool,uint64_t> bytesAppended = append(buf,count);
    if (bytesAppended.first==false)
    {
        ret.succeed=wrStatUpd.succeed=setFailedWriteState();
    }
    else
    {
        ret.bytesWritten+=bytesAppended.second;
        wrStatUpd.bytesWritten+=bytesAppended.second;
    }
    return ret;
}

BinaryContainer_Base *BinaryContainer_Base::copyToFS(const std::string &fileName, bool removeOnDestroy)
{
    BinaryContainer_MMAP * mmapbc = new BinaryContainer_MMAP();
    mmapbc->setFsBaseFileName(fsBaseFileName);
    mmapbc->setFsDirectoryPath(fsDirectoryPath);
    // TODO: passing filename when passing from chunks/mem to file.
    if (!mmapbc->referenceFile(fileName))
    {
        delete mmapbc;
        return nullptr;
    }
    mmapbc->setRemoveOnDestroy(removeOnDestroy);
    // dump this container into the mmaped binary container.
    std::pair<bool,uint64_t> bytesAppended = appendTo(*mmapbc);
    if (bytesAppended.second!=size() || !bytesAppended.first)
    {
        mmapbc->setRemoveOnDestroy(true);
        delete mmapbc;
        return nullptr;
    }
    return mmapbc;
}

std::string BinaryContainer_Base::getFsDirectoryPath() const
{
    return fsDirectoryPath;
}

void BinaryContainer_Base::setFsDirectoryPath(const std::string &value)
{
    fsDirectoryPath = value;
}

std::string BinaryContainer_Base::getFsBaseFileName() const
{
    return fsBaseFileName;
}

void BinaryContainer_Base::setFsBaseFileName(const std::string &value)
{
    fsBaseFileName = value;
}

std::string BinaryContainer_Base::getCurrentFileName() const
{
    return "";
}

std::pair<bool,uint64_t> BinaryContainer_Base::copyTo2(StreamableObject &bc, const uint64_t &bytes, const uint64_t &offset)
{
    WRStatus wrStatUpd;
    return copyTo2(bc,wrStatUpd,bytes,offset);
}

void BinaryContainer_Base::incContainerBytesCount(const uint64_t &i)
{
    setContainerBytes(size()+i);
}

void BinaryContainer_Base::decContainerBytesCount(const uint64_t &i)
{
    setContainerBytes(size()-i);
}

void BinaryContainer_Base::setContainerBytes(const uint64_t &value)
{
    containerBytes = value;
}

uint64_t BinaryContainer_Base::copyToStreamUsingCleanVector(std::ostream &bc, std::vector<BinaryContainerChunk> copyChunks)
{
    uint64_t dataCopied = 0;

    // Appending mode.
    for (size_t i=0; i<copyChunks.size();i++)
    {
        // TODO: Check the outcome of write...
        bc.write(copyChunks[i].rodata,copyChunks[i].rosize);
        dataCopied+= copyChunks[i].rosize;
    }

    return dataCopied;
}

uint64_t BinaryContainer_Base::copyToSOUsingCleanVector(StreamableObject &bc, std::vector<BinaryContainerChunk> copyChunks, WRStatus &wrStatUpd)
{
    WRStatus acum;
    // Appending mode.
    for (size_t i=0; i<copyChunks.size();i++)
    {
        WRStatus cur;
        if ( !(cur = bc.write(copyChunks[i].rodata,copyChunks[i].rosize,wrStatUpd)).succeed || cur.bytesWritten!=copyChunks[i].size)
        {
            acum+=cur;
            return acum.bytesWritten;
        }
        acum+=cur;
    }
    return acum.bytesWritten;
}
