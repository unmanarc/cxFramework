#ifndef BINARYCONTAINER_MMAP_H
#define BINARYCONTAINER_MMAP_H

#include "binarycontainer_base.h"
#include "binarycontainer_filereference.h"
#include "binarycontainer_mem.h"

class BinaryContainer_MMAP: public BinaryContainer_Base
{
public:
    BinaryContainer_MMAP();
    ~BinaryContainer_MMAP() override;
    // TODO: rename

    /**
     * @brief Reference a File in mmap mode.
     * @param filePath file to be referenced.
     * @return true if succeed
     */
    bool referenceFile(const std::string & filePath = "", bool readOnly = false, bool createFile = true);
    /**
     * @brief getCurrentFileName Get current FileName Full Path
     * @return Full path string
     */
    std::string getCurrentFileName() const override;

    /**
     * @brief setRemoveOnDestroy set remove file when this object is deleted
     * @param value true for remove the file.
     */
    void setRemoveOnDestroy(bool value);

    virtual uint64_t size() const override;
    /**
     * @brief findChar
     * @param c
     * @param offset
     * @return
     */
    std::pair<bool,uint64_t> findChar(const int & c, const uint64_t &offset = 0, uint64_t searchSpace = 0, bool caseSensitive = false) override;


protected:
    /**
     * @brief truncate the current container to n bytes.
     * @param bytes n bytes.
     * @return new container size.
     */
    std::pair<bool, uint64_t> truncate2(const uint64_t &bytes) override;
    /**
     * @brief Append more data
     * @param data data to be appended
     * @param len data size in bytes to be appended
     * @param prependMode mode: true will prepend the data, false will append.
     * @return appended bytes
     */
    std::pair<bool,uint64_t> append2(const void * buf, const uint64_t &len, bool prependMode = false) override;
    /**
     * @brief remove n bytes at the beggining shrinking the container
     * @param bytes bytes to be removed
     * @return bytes removed.
     */
    std::pair<bool, uint64_t> displace2(const uint64_t &bytes = 0) override;
    /**
     * @brief free the whole container
     * @return true if succeed
     */
    bool clear2() override;
    /**
    * @brief Append this current container to a stream.
    * @param out data stream out
    * @param bytes size of data to be copied in bytes. -1 copy all the container but the offset.
    * @param offset displacement in bytes where the data starts.
    * @return
    */
    std::pair<bool,uint64_t> copyToStream2(std::ostream & out, const uint64_t &bytes = std::numeric_limits<uint64_t>::max(), const uint64_t &offset = 0) override;
    /**
    * @brief Internal Copy function to copy this container to a new one.
    * @param bc Binary container.
    * @param bytes size of data to be copied in bytes. -1 copy all the container but the offset.
    * @param offset displacement in bytes where the data starts.
    * @return
    */
    std::pair<bool,uint64_t> copyTo2(StreamableObject & bc, WRStatus &wrStatUpd, const uint64_t &bytes = std::numeric_limits<uint64_t>::max(), const uint64_t &offset = 0) override;
    /**
     * @brief Copy append to another binary container.
     * @param bc destination binary container
     * @param bytes size of data in bytes to be copied
     * @param offset starting point (offset) in bytes, default: 0 (start)
     * @return number of bytes copied (in bytes)
     */
    std::pair<bool, uint64_t> copyOut2(void * buf, const uint64_t &count, const uint64_t &offset = 0) override;
    /**
     * @brief Compare memory with the container
     * @param mem Memory to be compared
     * @param len Memory size in bytes to be compared
     * @param offset starting point (offset) in bytes, default: 0 (start)
     * @return true where comparison returns equeal.
     */
    bool compare2(const void * buf, const uint64_t &count, bool caseSensitive = true, const uint64_t &offset = 0) override;

private:
    void reMapMemoryContainer();

    /**
     * @brief getRandomFileName Create a new random fileName
     * @return random filename with path
     */
    std::string getRandomFileName();
    /**
     * @brief Creates empty file on the filesystem to use it in future.
     * @return true if the file was successfully created at the specified fileName (or produced one)
     */
    bool createEmptyFile(const std::string & fileName);


    // File:
    BCFileReference fileReference;
    BinaryContainer_MEM mem;
};

#endif // BINARYCONTAINER_MMAP_H
