#ifndef BINARYCONTAINER_BASE_H
#define BINARYCONTAINER_BASE_H

#include "common.h"
#include "binarycontainer_chunk.h"

#include <cx_mem_streams/streamableobject.h>

#include <limits.h>
#include <stdio.h>
#include <list>
#include <vector>
#include <iostream>

enum BinaryContainerMethod {
    BC_METHOD_CHUNKS,
    BC_METHOD_MEM,
    BC_METHOD_BCREF,
    BC_METHOD_FILEMMAP,
    BC_METHOD_NULL
};

class BinaryContainer_Base : public StreamableObject
{
public:
    BinaryContainer_Base();
    virtual ~BinaryContainer_Base() override;
    /**
     * @brief operator = copy the data of a container into another container (does not copy limits or flags like read only)
     * @param bc element to be copied.
     */
    BinaryContainer_Base & operator=(BinaryContainer_Base & bc);

    /**
     * @brief print print to stdout...
     */
    void print(FILE * f = stdout);

    // Data Agregattion method method:
    /**
     * @brief Prepend linear memory here.
     * @param bc Binary container.
     * @return bytes prepended
     */
    std::pair<bool,uint64_t> prepend(const void * buf);
    /**
     * @brief Append null terminated linear memory to this container.
     * @param data null terminated linear memory data.
     * @return bytes appended
     */
    std::pair<bool,uint64_t> append(const void * buf);
    /**
    * @brief Append linear memory to this container.
    * @param data linear memory data.
    * @param len linear memory data size in bytes.
    * @return bytes appended
    */
    std::pair<bool,uint64_t> append(const void * buf, uint64_t len);
    /**
     * @brief Prepend linear memory here.
     * @param bc Binary container.
     * @return bytes prepended
     */
    std::pair<bool,uint64_t> prepend(const void * buf, uint64_t len);
    // Memory shrinking..
    /**
     * @brief remove n bytes at the beggining shrinking the container
     * @param bytes bytes to be removed
     * @return bytes removed.
     */
    std::pair<bool,uint64_t> displace(const uint64_t &bytes = 0);
    /**
     * @brief truncate the current container to n bytes.
     * @param bytes n bytes.
     * @return new container size.
     */
    std::pair<bool, uint64_t> truncate(const uint64_t &bytes);
    /**
     * @brief free the whole container
     * @return true if succeed
     */
    bool clear();
    // Copy mechanisms:
    /**
     * @brief copy Until some byte sequence
     * @param destination Binary container where the resulted data will be saved.
     * @param needle delimiter
     * @param needle_len delimiter length
     * @param maxCopySize maximum bytes to be retrieved in bytes.
     * @return retr Return codes [0:found, -1:failed, not found, -2:failed, out of size]
     */
    int copyUntil(BinaryContainer_Base & destination, const void * needle, const size_t &needleLenght, const uint64_t &maxCopySize, bool removeNeedle = false );
    /**
     * @brief displaceUntil move until some byte sequence
     * @param destination Binary container where the resulted data will be saved.
     * @param needle delimiter
     * @param needleCount delimiter length in bytes
     * @param maxCopySize maximum bytes to be retrieved in bytes.
     * @return retr return codes [0:found, -1:failed, not found, -2:failed, out of size]
     */
    int displaceUntil(BinaryContainer_Base & destination, const void * needle, const size_t &needleCount, const uint64_t &maxCopySize, bool removeNeedle = false );
    /**
     * @brief displaceUntil move until some byte sequence
     * @param needle delimiter
     * @param needle_len delimiter length in bytes
     * @param maxCopySize maximum bytes to be retrieved in bytes.
     * @return retr return codes [0:found, -1:failed, not found, -2:failed, out of size]
     */
    int displaceUntil(BinaryContainer_Base & destination, const std::list<std::string> needles, const uint64_t &maxCopySize, bool removeNeedle = true );
    /**
     * @brief referencedSplit split the current container into many referencers.
     * @param needles needles to be used
     * @param maxSearchSize Maximum search size per needle.
     * @param maxNeedles Maximum needles to be searched.
     * @return list of binary containers referencing this container (you should delete them)
     */
  //  std::list<BinaryContainer_Base *> referencedSplit(const std::list<std::string> & needles,  const uint64_t & maxSearchSize=0, const size_t &maxNeedles=0);
    /**
     * @brief referencedSplit split the current container into many referencers.
     * @param needles needles to be used
     * @param maxSearchSize Maximum search size per needle.
     * @param maxNeedles Maximum needles to be searched.
     * @return list of binary containers referencing this container (you should delete them)
     */
 //   std::list<BinaryContainer_Base *> referencedSplit2(const std::list<std::string> & needles, const std::list<std::string> & skipBegWith, const uint64_t & maxSearchSize, const size_t & maxNeedles);
    /**
     * @brief freeSplitList free list of binary container
     * @param x
     */
    static void freeSplitList(std::list<BinaryContainer_Base *> x);
    /**
    * @brief Append this current container to a new one.
    * @param out Binary container.
    * @param bytes size of data to be copied in bytes. -1 copy all the container but the offset.
    * @param offset displacement in bytes where the data starts.
    * @return
    */
    std::pair<bool, uint64_t> copyToStream(std::ostream &out, uint64_t bytes = std::numeric_limits<uint64_t>::max(), const uint64_t &offset = 0);
    /**
    * @brief Append this current container to a new one. (without reporting EOF)
    * @param bc Binary container.
    * @param bytes size of data to be copied in bytes. -1 copy all the container but the offset.
    * @param offset displacement in bytes where the data starts.
    * @return -1 if error, 0 if no data appended (eg. max reached), n bytes appended.
    */
    std::pair<bool,uint64_t> appendTo(StreamableObject & out, const uint64_t &bytes = std::numeric_limits<uint64_t>::max(), const uint64_t &offset = 0);
    std::pair<bool,uint64_t> appendTo(StreamableObject & out, WRStatus & wrStatUpd, uint64_t bytes = std::numeric_limits<uint64_t>::max(), const uint64_t &offset = 0);
    /**
     * @brief Copy append to another binary container.
     * @param bc destination binary container
     * @param bytes size of data in bytes to be copied
     * @param offset starting point (offset) in bytes, default: 0 (start)
     * @return number of bytes copied (in bytes)
     */
    std::pair<bool,uint64_t> copyOut(void * buf, uint64_t bytes, const uint64_t &offset = 0);
    /**
     * @brief Copy the container to an std::string
     * @param bytes bytes to copy (std::numeric_limits<uint64_t>::max(): all bytes)
     * @param offset offset displacement
     * @return bytes copied or std::numeric_limits<uint64_t>::max() if error.
     */
    std::pair<bool,uint64_t> copyToString(std::string & str,uint64_t bytes = std::numeric_limits<uint64_t>::max(), const uint64_t & offset = 0);

    /**
     * @brief toString create string with the data contained here.
     * @param bytes bytes to copy (std::numeric_limits<uint64_t>::max(): all bytes)
     * @param offset offset displacement
     * @return string containing the data.
     */
    std::string toString(uint64_t bytes = std::numeric_limits<uint64_t>::max(), const uint64_t &offset = 0);
    /**
     * @brief toUInt64 Convert container data to 64-bit unsigned integer
     * @param base eg. 10 for base10 numeric, and 16 for hex
     * @param bytes bytes to copy (std::numeric_limits<uint64_t>::max(): all bytes)
     * @param offset offset displacement
     * @return std::numeric_limits<uint64_t>::max() if error, n if converted.
     */
    uint64_t toUInt64(int base=10, const uint64_t &bytes = std::numeric_limits<uint64_t>::max(), const uint64_t &offset = 0);
    /**
     * @brief toUInt32 Convert container data to 32-bit unsigned integer
     * @param base eg. 10 for base10 numeric, and 16 for hex
     * @param bytes bytes to copy (std::numeric_limits<uint64_t>::max(): all bytes)
     * @param offset offset displacement
     * @return ULONG_MAX if error, n if converted.
     */
    uint32_t toUInt32(int base=10, const uint64_t &bytes = std::numeric_limits<uint64_t>::max(), const uint64_t &offset = 0);

    // Needles / Comparison:
    /**
     * @brief Compare memory with the container
     * @param mem Memory to be compared
     * @param len Memory size in bytes to be compared
     * @param offset starting point (offset) in bytes, default: 0 (start)
     * @return true where comparison returns equeal.
     */
    bool compare(const void * mem, const uint64_t &len, bool caseSensitive = true, const uint64_t &offset = 0 );
    /**
     * @brief Compare memory with the container
     * @param cmpString string to be compared
     * @param caseSensitive do a case sensitive comparison
     * @param offset starting point (offset) in bytes, default: 0 (start)
     * @return true where comparison returns equeal.
     */
    bool compare(const std::string & cmpString, bool caseSensitive = false, const uint64_t &offset = 0 );

    /**
     * @brief findChar get the position of character from the beggining of the container.
     * @param c character to find.
     * @param offset offset bytes to discard.
     * @param searchSpace search size from offset
     * @return
     */
    virtual std::pair<bool,uint64_t> findChar(const int & c, const uint64_t &offset = 0, uint64_t searchSpace = 0, bool caseSensitive = false) = 0;

    /**
     * @brief find memory into the container
     * @param needle memory data to be found.
     * @param len memory data to be found size in bytes.
     * @param offset container offset where to start to find.
     * @param searchSpace search space size in bytes where is going to find the needle. (zero for all the space)
     * @return position of the needle (if found)
     */
    std::pair<bool,uint64_t> find(const void * needle, const size_t &needle_len, bool caseSensitive = true, const uint64_t &offset = 0, uint64_t searchSpace = 0);
    /**
     * @brief find memory into the container
     * @param needle memory data to be found.
     * @param len memory data to be found size in bytes.
     * @param offset container offset where to start to find.
     * @param searchSpace search space size in bytes where is going to find the needle. (zero for all the space)
     * @return position of the needle (if found)
     */
    std::pair<bool, uint64_t> find(const std::list<std::string> &needles, std::string & needleFound, bool caseSensitive = true, const uint64_t &offset = 0, const uint64_t &searchSpace = 0);

    // Data Size:
    /**
     * @brief size Get Container Data Size in bytes
     * @return data size in bytes
     */
    virtual uint64_t size() const override;
    /**
     * @brief Is the container Null
     * @return true if container size is 0.
     */
    bool isNull();
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Container behaviour.
    /**
     * @brief Get Maximum Container Size allowed in bytes
     * @return Maximum Container Size allowed in bytes
     */
    uint64_t getMaxSize() const;
    /**
     * @brief Set Maximum Container Size allowed in bytes
     * @param value Maximum Container Size allowed in bytes
     */
    void setMaxSize(const uint64_t &value);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // StreamableObject
    bool streamTo(StreamableObject * out, WRStatus & wrStatUpd) override;
    WRStatus write(const void * buf, const size_t &count, WRStatus & wrStatUpd) override;
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // FS options:
    BinaryContainer_Base * copyToFS(const std::string &fileName, bool removeOnDestroy);

    /**
     * @brief get the directory path where the temporary file will be saved
     * @return directory path
     */
    std::string getFsDirectoryPath() const;
    /**
     * @brief set the directory path where the temporary file will be saved
     * @param value directory path (eg. /tmp)
     */
    void setFsDirectoryPath(const std::string &value);
    /**
     * @brief getFsBaseFileName
     * @return
     */
    std::string getFsBaseFileName() const;
    /**
     * @brief setFsBaseFileName
     * @param value
     */
    void setFsBaseFileName(const std::string &value);
    /**
     * @brief getCurrentFileName Get current FileName Full Path
     * @return Full path string
     */
    virtual std::string getCurrentFileName() const;


protected:
    /**
     * @brief truncate the current container to n bytes.
     * @param bytes n bytes.
     * @return new container size.
     */
    virtual std::pair<bool, uint64_t> truncate2(const uint64_t &bytes) = 0;

    /**
     * @brief free the whole container
     * @return true if succeed
     */
    virtual bool clear2() = 0;
    /**
     * @brief remove n bytes at the beggining shrinking the container
     * @param bytes bytes to be removed
     * @return bytes removed.
     */
    virtual std::pair<bool,uint64_t> displace2(const uint64_t &bytes = 0) = 0;
    /**
     * @brief Append more data to current chunks. (creates new chunks of data)
     * @param data data to be appended
     * @param len data size in bytes to be appended
     * @param prependMode mode: true will prepend the data, false will append.
     * @return true if succeed
     */
    virtual std::pair<bool,uint64_t> append2(const void * buf, const uint64_t &len, bool prependMode) = 0;
    /**
    * @brief Internal Copy function to copy this container to a stream
    * @param out data stream out
    * @param bytes size of data to be copied in bytes. -1 copy all the container but the offset.
    * @param offset displacement in bytes where the data starts.
    * @return
    */
    virtual std::pair<bool,uint64_t> copyToStream2(std::ostream & out, const uint64_t &bytes = std::numeric_limits<uint64_t>::max(), const uint64_t &offset = 0) = 0;
    /**
    * @brief Internal Copy function to copy this container to a new one.
    * @param out data stream out
    * @param bytes size of data to be copied in bytes. -1 copy all the container but the offset.
    * @param offset displacement in bytes where the data starts.
    * @return number of bytes copied
    */
    virtual std::pair<bool,uint64_t> copyTo2(StreamableObject & bc, const uint64_t &bytes = std::numeric_limits<uint64_t>::max(), const uint64_t &offset = 0);
    virtual std::pair<bool,uint64_t> copyTo2(StreamableObject & bc, WRStatus & wrStatUpd, const uint64_t &bytes = std::numeric_limits<uint64_t>::max(), const uint64_t &offset = 0) = 0;
    /**
     * @brief Copy append to external memory
     * @param bc destination binary container
     * @param bytes size of data in bytes to be copied
     * @param offset starting point (offset) in bytes, default: 0 (start)
     * @return number of bytes copied
     */
    virtual std::pair<bool,uint64_t> copyOut2(void * buf, const uint64_t &bytes, const uint64_t &offset = 0) = 0;
    /**
     * @brief Compare memory with the container
     * @param mem Memory to be compared
     * @param len Memory size in bytes to be compared
     * @param offset starting point (offset) in bytes, default: 0 (start)
     * @return true where comparison returns equeal.
     */
    virtual bool compare2(const void * buf, const uint64_t &len, bool caseSensitive = true, const uint64_t &offset = 0 ) = 0;


    /**
     * @brief addToContainerBytes Internal function to increase container bytes count.
     * @param i
     */
    void incContainerBytesCount(const uint64_t & i);
    /**
     * @brief remFromContainerBytes Internal function to decrease container bytes count.
     * @param i
     */
    void decContainerBytesCount(const uint64_t & i);

    // Auxiliar:
    uint64_t copyToStreamUsingCleanVector(std::ostream &bc, std::vector<BinaryContainerChunk> copyChunks);
    uint64_t copyToSOUsingCleanVector(StreamableObject &bc, std::vector<BinaryContainerChunk> copyChunks, WRStatus & wrStatUpd);
    void setContainerBytes(const uint64_t &value);

    ///////////////////////////////////////////////////////////////////////////////////////////////////////
    // Variables::
    /**
     * @brief readOnly defined if it's in read-only mode or not.
     */
    bool readOnly;
    /**
     * @brief containerBytes container current size in bytes.
     */
    uint64_t containerBytes;
    /**
     * @brief maxSize Maximum size of the container
     */
    uint64_t maxSize;

    // Storage Method:
    /**
     * @brief storeMethod Storage Mechanism used (read only memory reference, chunks, file)
     */
    BinaryContainerMethod storeMethod;

    // FS directives:
    /**
     * @brief fsDirectoryPath Directory where files will be created.
     */
    std::string fsDirectoryPath;
    /**
     * @brief fsBaseFileName base filename on the directory.
     */
    std::string fsBaseFileName;

private:
    bool clear0();

};

#endif // BINARYCONTAINER_BASE_H
