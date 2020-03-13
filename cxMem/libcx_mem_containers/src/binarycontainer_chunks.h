#ifndef BINARYCONTAINER_CHUNKS_H
#define BINARYCONTAINER_CHUNKS_H

#include "binarycontainer_base.h"
#include "binarycontainer_mmap.h"

#include <vector>

class BinaryContainer_Chunks : public BinaryContainer_Base
{
public:
    BinaryContainer_Chunks();
    ~BinaryContainer_Chunks() override;

    void setMaxContainerSizeUntilGoingToFS(const uint64_t &value);
    uint64_t getMaxContainerSizeUntilGoingToFS() const;

    /**
     * @brief isUsingFiles is using mmap
     * @return true if using mmap
     */
    bool isUsingFiles();
    /**
     * @brief getMmapContainer get the container who manage the file (useful for stopping it from delete the file when finish)
     * @return pointer to the mmap container.
     */
    BinaryContainer_MMAP *getMmapContainer() const;

    // Specific chunks options.
    /**
     * @brief Get Max Number of Chunks in Memory
     * @return Max Number of Chunks in Memory
     */
    size_t getMaxChunks() const;
    /**
     * @brief Set Max Number of Chunks in Memory
     * @param value Max Number of Chunks in Memory
     */
    void setMaxChunks(const size_t &value);
    /**
     * @brief setMaxChunkSize Set Maximum Chunk Size on BC_METHOD_CHUNKS
     * @param value Max Chunk size in bytes.
     */
    void setMaxChunkSize(const uint32_t & value);
    /**
     * @brief size Get Container Data Size in bytes
     * @return data size in bytes
     */
    virtual uint64_t size() const override;
    /**
     * @brief findChar
     * @param c
     * @param offset
     * @return
     */
    std::pair<bool,uint64_t> findChar(const int &c, const uint64_t &roOffset = 0, uint64_t  searchSpace = 0, bool caseSensitive = false) override;


protected:
    /**
     * @brief truncate the current container to n bytes.
     * @param bytes n bytes.
     * @return new container size.
     */
    std::pair<bool,uint64_t> truncate2(const uint64_t &bytes) override;
    /**
     * @brief Append more data to current chunks. (creates new chunks of data)
     * @param data data to be appended
     * @param len data size in bytes to be appended
     * @param prependMode mode: true will prepend the data, false will append.
     * @return true if succeed
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
    * @brief Append this current container to a new one.
    * @param bc Binary container.
    * @param bytes size of data to be copied in bytes. -1 copy all the container but the offset.
    * @param offset displacement in bytes where the data starts.
    * @return
    */
    std::pair<bool,uint64_t> copyToStream2(std::ostream & bc, const uint64_t &bytes = std::numeric_limits<uint64_t>::max(), const uint64_t &offset = 0) override;
    /**
    * @brief Internal Copy function to copy this container to a new one.
    * @param out data stream out
    * @param bytes size of data to be copied in bytes. -1 copy all the container but the offset.
    * @param offset displacement in bytes where the data starts.
    * @return
    */
    std::pair<bool,uint64_t> copyTo2(StreamableObject & bc, WRStatus & wrStatUpd, const uint64_t &bytes = std::numeric_limits<uint64_t>::max(), const uint64_t &offset = 0) override;
    /**
     * @brief Copy append to another binary container.
     * @param bc destination binary container
     * @param bytes size of data in bytes to be copied
     * @param offset starting point (offset) in bytes, default: 0 (start)
     * @return number of bytes copied (in bytes)
     */
    std::pair<bool,uint64_t> copyOut2(void * buf, const uint64_t &bytes, const uint64_t &offset = 0) override;
    /**
     * @brief Compare memory with the container
     * @param mem Memory to be compared
     * @param len Memory size in bytes to be compared
     * @param offset starting point (offset) in bytes, default: 0 (start)
     * @return true where comparison returns equeal.
     */
    bool compare2(const void * buf, const uint64_t &len, bool caseSensitive = true, const uint64_t &offset = 0 ) override;

private:

    bool clearMmapedContainer();
    bool clearChunks();

    /**
     * @brief Recalculates every offset on the list
     */
    void recalcChunkOffsets();
    /**
     * @brief getChunkForOffset Get chunk containing offset
     * @param offset offset from zero on binarycontainer.
     * @return -1 if not found, or the vector position of the chunk.
     */
    size_t I_Chunk_GetPosForOffset(const uint64_t &offset, size_t curpos = MAX_SIZE_T, size_t curmax = MAX_SIZE_T, size_t curmin = MAX_SIZE_T);

    /**
     * @brief chunksVector defines a vector containing ordered chunks
     */
    std::vector<BinaryContainerChunk> chunksVector;
    /**
     * @brief Max number of Chunks in memory
     */
    size_t maxChunks;
    /**
     * @brief maxChunkSize Maximum Chunk Size.
     */
    uint32_t maxChunkSize;
    /**
     * @brief Maximum Container Size in bytes Until Swapping to FS
     */
    uint64_t maxContainerSizeUntilGoingToFS;
    /**
     * @brief mmapContainer
     */
    BinaryContainer_Base * mmapContainer;
};

#endif // BINARYCONTAINER_CHUNKS_H
