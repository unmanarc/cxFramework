#ifndef BINARYCONTAINER_REF_H
#define BINARYCONTAINER_REF_H

#include "binarycontainer_base.h"

class BinaryContainer_Ref : public BinaryContainer_Base
{
public:
    BinaryContainer_Ref(BinaryContainer_Base *bc = nullptr, const uint64_t &offset=0, const uint64_t &maxBytes=0);
    ~BinaryContainer_Ref() override;
    /**
     * @brief reference another binary container into this container.
     * @param bc binary container to be referenced.
     * @param offset start position.
     * @param maxBytes max bytes to be referenced (std::numeric_limits<uint64_t>::max(): unlimited, but if maxBytes>0, it will be readOnly)
     */
    bool reference(BinaryContainer_Base * bc, const uint64_t &offset=0, const uint64_t &maxBytes = std::numeric_limits<uint64_t>::max());
    /**
     * @brief getReferencedBC Get referenced object
     * @return referenced object pointer.
     */
    BinaryContainer_Base *getReferencedBC() const;
    /**
     * @brief size Get Container Data Size in bytes
     * @return data size in bytes
     */
    uint64_t size() const override;
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
    std::pair<bool,uint64_t> truncate2(const uint64_t &bytes) override;
    /**
     * @brief Append is disabled.
     * @return 0.
     */
    std::pair<bool,uint64_t> append2(const void * buf, const uint64_t &len, bool prependMode = false) override;
    /**
     * @brief remove n bytes at the beggining shrinking the container
     * @param bytes bytes to be removed
     * @return bytes removed.
     */
    std::pair<bool,uint64_t> displace2(const uint64_t &bytes = 0) override;
    /**
     * @brief free the whole container
     * @return true if succeed
     */
    bool clear2() override;
    /**
    * @brief Append this current container to a stream.
    * @param bc Binary container.
    * @param bytes size of data to be copied in bytes. -1 copy all the container but the offset.
    * @param offset displacement in bytes where the data starts.
    * @return
    */
    std::pair<bool,uint64_t> copyToStream2(std::ostream & out, const uint64_t &bytes = std::numeric_limits<uint64_t>::max(), const uint64_t &offset = 0) override;
    /**
    * @brief Internal Copy function to copy this container to a new one.
    * @param out data stream out
    * @param bytes size of data to be copied in bytes. -1 copy all the container but the offset.
    * @param offset displacement in bytes where the data starts.
    * @return
    */
    std::pair<bool,uint64_t> copyTo2(StreamableObject &bc, WRStatus &wrStatUpd, const uint64_t &bytes = std::numeric_limits<uint64_t>::max(), const uint64_t &offset = 0) override;
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
    /**
     * @brief referencedBC referenced Binary container (then, this container will not work autonomous)
     */
    BinaryContainer_Base * referencedBC;
    /**
     * @brief referecedOffset binary container reference offset in bytes
     */
    uint64_t referencedOffset;
    /**
     * @brief referencedMaxBytes binary container reference bytes to be referenced
     */
    uint64_t referencedMaxBytes;

};

#endif // BINARYCONTAINER_REF_H
