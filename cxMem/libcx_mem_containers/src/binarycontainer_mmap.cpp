#include "binarycontainer_mmap.h"
#include <time.h>
#include <sys/timeb.h>
#include <random>

BinaryContainer_MMAP::BinaryContainer_MMAP()
{
    storeMethod = BC_METHOD_FILEMMAP;
    readOnly = false;
    setContainerBytes(0);
    BinaryContainer_MMAP::clear2();
}

BinaryContainer_MMAP::~BinaryContainer_MMAP()
{
    BinaryContainer_MMAP::clear2();
}

bool BinaryContainer_MMAP::referenceFile(const std::string &filePath, bool readOnly, bool createFile)
{
    BinaryContainer_MMAP::clear2();
    BCFileReference bcr;

    if (filePath == "")
    {
        bcr.currentFileName = getRandomFileName();
    }
    else
    {
        bcr.currentFileName = filePath;
    }

    // Open the file:
    if (!bcr.openFile(bcr.currentFileName, readOnly, createFile))
    {
        // Failed to open the referenced file.

        /*        if () O_CREAT should do the job for us.
        // Create one.
        if (!createEmptyFile(bcr.currentFileName)) return false;

        // Open it again.
        if (!bcr.openFile(bcr.currentFileName)) return false;*/
        return false;
    }

    // Destroy current memory and assign reference values:
    mem.reference(bcr.mmapAddr,bcr.fileOpenSize);

    // Copy file descriptor and other things.
    fileReference = bcr;

    // prevent bcr from destroying the descriptor on exit:
    bcr.fd = -1;

    return true;
}

void BinaryContainer_MMAP::setRemoveOnDestroy(bool value)
{
    fileReference.removeOnDestroy = value;
}

uint64_t BinaryContainer_MMAP::size() const
{
    // TODO: check
//    std::cout << "BinaryContainer_MMAP::  Getting size() " << mem.size() << std::endl << std::flush;
    return mem.size();
}

std::pair<bool, uint64_t> BinaryContainer_MMAP::findChar(const int &c, const uint64_t &offset, uint64_t searchSpace, bool caseSensitive)
{
    if (caseSensitive && !(c>='A' && c<='Z') && !(c>='a' && c<='z') )
        caseSensitive = false;
    return mem.findChar(c,offset,searchSpace, caseSensitive);
}

std::pair<bool, uint64_t> BinaryContainer_MMAP::truncate2(const uint64_t &bytes)
{
    if (!fileReference.mmapTruncate(bytes))
    {
        clear();
        return std::make_pair(false,(uint64_t)0);
    }

    reMapMemoryContainer();
    return std::make_pair(false,size());
}

std::string BinaryContainer_MMAP::getCurrentFileName() const
{
    return fileReference.currentFileName;
}

std::pair<bool, uint64_t> BinaryContainer_MMAP::append2(const void *buf, const uint64_t &len, bool prependMode)
{
    std::pair<bool, uint64_t> addedBytes;

    if (prependMode)
        addedBytes = fileReference.mmapPrepend(buf,len);
    else
        addedBytes = fileReference.mmapAppend(buf,len);

    if (!addedBytes.first)
    {
        //clear(); // :(
        return addedBytes;
    }

    reMapMemoryContainer();

    return addedBytes;
}

std::pair<bool, uint64_t> BinaryContainer_MMAP::displace2(const uint64_t &roBytesToDisplace)
{
    uint64_t bytesToDisplace = roBytesToDisplace;

    if (bytesToDisplace>fileReference.fileOpenSize)
        return std::make_pair(false,(uint64_t)0);
    if (!fileReference.mmapDisplace(bytesToDisplace))
        return std::make_pair(false,(uint64_t)0);

    reMapMemoryContainer();
    return std::make_pair(true,bytesToDisplace);
}

bool BinaryContainer_MMAP::clear2()
{
    return fileReference.closeFile();
}

std::pair<bool, uint64_t> BinaryContainer_MMAP::copyToStream2(std::ostream &out, const uint64_t &bytes, const uint64_t &offset)
{
    return mem.copyToStream(out,bytes,offset);
}

std::pair<bool, uint64_t> BinaryContainer_MMAP::copyTo2(StreamableObject &bc, WRStatus & wrStatUpd, const uint64_t &bytes, const uint64_t &offset)
{
    return mem.appendTo(bc,wrStatUpd,bytes,offset);
}

std::pair<bool,uint64_t> BinaryContainer_MMAP::copyOut2(void *buf, const uint64_t &count, const uint64_t &offset)
{
    return mem.copyOut(buf,count,offset);
}

bool BinaryContainer_MMAP::compare2(const void *buf, const uint64_t &count, bool caseSensitive, const uint64_t &offset)
{
    return this->mem.compare(buf,count,caseSensitive,offset);
}

void BinaryContainer_MMAP::reMapMemoryContainer()
{
    setContainerBytes(fileReference.fileOpenSize);
    mem.reference(fileReference.mmapAddr,size());
}

std::string BinaryContainer_MMAP::getRandomFileName()
{
    std::string::size_type length = 16;
    char baseChars[] = "abcdefghijklmnopqrstuvwxyz0123456789";
    std::string randomStr;
    std::mt19937 rg{std::random_device{}()};
    std::uniform_int_distribution<std::string::size_type> pick(0, sizeof(baseChars)-2);
    randomStr.reserve(length);
    while(length--) randomStr += baseChars[pick(rg)];

    return fsDirectoryPath + "/" + fsBaseFileName + "." + randomStr;
}

bool BinaryContainer_MMAP::createEmptyFile(const std::string &)
{
    // Creates file here.
    BCFileReference bcr;
    // TODO:
    /*
    if (!access(fileName.c_str(), F_OK)) return false;

    // Open the file:
    if (!bcr.openFile(fileName))
    {
        // Failed to open the referenced file.
        return false;
    }

    // Don't remove/close on destroy... ;)...
    bcr.fd = -1;

    return true;*/
    return false;
}

