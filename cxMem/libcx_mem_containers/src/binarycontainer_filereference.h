#ifndef BINARYCONTAINERFILEREFERENCE_H
#define BINARYCONTAINERFILEREFERENCE_H

#include <string>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/mman.h>

#include "common.h"

struct BCFileReference {

    BCFileReference()
    {
        removeOnDestroy = false;
        fd = -1;
        mmapAddr = nullptr;
        fileOpenSize=0;
    }
    ~BCFileReference()
    {
        if (fd>=0)
        {
            closeFile();
        }
    }

    void cleanVars()
    {
        currentFileName = "";
        removeOnDestroy = false;
        fd = -1;
        mmapAddr = nullptr;
        fileOpenSize=0;
    }

    bool mmapDisplace(const uint64_t &offsetBytes)
    {
        memmove64(mmapAddr, mmapAddr+offsetBytes, fileOpenSize-offsetBytes);
        return mmapTruncate(fileOpenSize-offsetBytes);
    }

    // Mmap/FILE MODE methods:
    // TODO: use mmap64... or plain seek mode...
    bool mmapTruncate(const uint64_t &nSize)
    {
        if (mmapAddr)
        {
            if (munmap(mmapAddr,fileOpenSize)!=0) return false;
            mmapAddr = nullptr;
        }

        fileOpenSize = nSize;
        if (ftruncate64(fd,fileOpenSize)!=0) return false;

        // Re-Map this.
        mmapAddr = static_cast<char *>(mmap(nullptr, fileOpenSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0));
        if (mmapAddr == MAP_FAILED)
        {
            mmapAddr = nullptr;
            return false;
        }
        return true;
    }

    // thanks to larsmans: http://stackoverflow.com/questions/4460507/appending-to-a-memory-mapped-file
    /**
     * @brief mmapAppend Append data to the mmap memory.
     * @param buf data to be appended
     * @param count bytes to be appended.
     * @return -1 if error, or bytes written (can be zero or different to nbytes).
     */
    std::pair<bool, uint64_t> mmapAppend(void const *buf, const uint64_t &count)
    {
        if (!count) return std::make_pair(true,0);
        /////////////////////////////

        uint64_t curOpenSize = fileOpenSize;
        if (!mmapTruncate(fileOpenSize+count)) return std::make_pair(false,(uint64_t)0);

        memcpy(mmapAddr+curOpenSize,buf,count);
        return std::make_pair(true,count);
    }

    /**
     * @brief mmapPrepend Prepend data to the mmap memory.
     * @param buf data to be prepended
     * @param count bytes to be preapended.
     * @return -1 if error, or bytes written (can be zero or different to nbytes).
     */
    std::pair<bool, uint64_t> mmapPrepend(void const *buf, const uint64_t &count)
    {
        if (!count) return std::make_pair(true,0);

        /////////////////////////////
        uint64_t curOpenSize = fileOpenSize;
        if (!mmapTruncate(fileOpenSize+count)) return std::make_pair(false,(uint64_t)0);
        memmove64(mmapAddr,mmapAddr+count,curOpenSize);
        memcpy64(mmapAddr,buf,count);
        return std::make_pair(true,count);
    }

    /**
     * @brief closeFile Close currently openned file.
     * @param respectRemoveOnDestroy if false, will not destroy the file at all.
     * @return true if close suceeded
     */
    bool closeFile(bool respectRemoveOnDestroy=true)
    {
        if (fd==-1) return true;

        if (mmapAddr) munmap(mmapAddr,fileOpenSize);

        close(fd);

        if (removeOnDestroy && respectRemoveOnDestroy && currentFileName.size())
        {
            remove(currentFileName.c_str());
        }

        cleanVars();
        return true;
    }

    bool openFile(const std::string & filePath, bool readOnly, bool createFile)
    {
        struct stat64 sbuf;
        int oflags = readOnly? O_RDONLY : (createFile? O_RDWR | O_APPEND | O_CREAT : O_RDWR | O_APPEND );
        if ((fd = open(filePath.c_str(), oflags, 0600)) == -1)
        {
            cleanVars();
            return false;
        }

        currentFileName = filePath;

        if (stat64(filePath.c_str(), &sbuf) == -1)
        {
            closeFile();
            return false;
        }
        fileOpenSize = sbuf.st_size;

        if (sbuf.st_size == 0)
        {
            // No map for zero bytes!
            mmapAddr = nullptr;
        }
        else
        {
            //printf("file %s openned with size : %llu\n", filePath.c_str(), fileOpenSize); fflush(stdout);

            int prot = readOnly? PROT_READ : PROT_READ | PROT_WRITE;
            mmapAddr = static_cast<char *>(mmap(nullptr, sbuf.st_size, prot, MAP_SHARED, fd, 0));
            if (mmapAddr == MAP_FAILED)
            {
                //printf("mmap failed!\n"); fflush(stdout);

                closeFile();
                return false;
            }
        }

        return true;
    }

    /**
     * @brief currentFileName current filename used.
     */
    std::string currentFileName;
    /**
     * @brief removeOnDestroy Remove the file when this class ends.
     */
    bool removeOnDestroy;
    /**
     * @brief fd mmap file descriptor:
     */
    int fd;
    /**
     * @brief linearMemOriginalPointer original pointer used
     */
    char * mmapAddr;
    /**
     * @brief containerBytesOriginalBytes original container size.
     */
    uint64_t fileOpenSize;
};

#endif // BINARYCONTAINERFILEREFERENCE_H

