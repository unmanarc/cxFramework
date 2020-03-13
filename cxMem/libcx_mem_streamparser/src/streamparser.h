#ifndef INTERNALMAINPARSER_H
#define INTERNALMAINPARSER_H

#include "substreamparser.h"

#include <cx_mem_containers/binarycontainer_chunks.h>
//#include <cx_net_sockets/socket_base_stream.h>
//#include <cx_net_chains/socketchainbase.h>

#include <cx_mem_streams/streamableobject.h>

enum ParseErrorMSG {
    PROT_PARSER_SUCCEED = 0,
    PROT_PARSER_ERR_INIT = -1,
    PROT_PARSER_ERR_READ = -2,
    PROT_PARSER_ERR_PARSE = -3
};

/**
 * @brief The ProtocolParser_Base class
 *
 * Via Object:
 *                                             *------------------*
 * Current Parser <---- Binary <--- Parser <---| StreamableObject |
 *                      Object                 *------------------*
 *
 * Via SO: (you can use writeStream and streamTo)
 *
 * Current Parser <---- Binary <--- Parser (StreamableObject)
 *                      Object
 *
 *
 */
class StreamParser : public StreamableObject
{
public:
    StreamParser(StreamableObject *value, bool clientMode);
    virtual ~StreamParser() override;

    /**
     * @brief parseObject Parse streamable object (init/parse/end)
     * @param err: (0:succeed, -1:failed to initialize, -2:failed to read, -3:failed to parse/write)
     * @return bytes written.
     */
    WRStatus parseObject(ParseErrorMSG * err);

    //////////////////////////////////////////

    virtual bool streamTo(StreamableObject * out, WRStatus & wrsStat) override;
    virtual WRStatus write(const void * buf, const size_t &count, WRStatus &wrStat) override;


    /**
     * @brief writeEOF Receive this when the connection is ended.
     */
    virtual void writeEOF(bool) override;

    //////////////////////////////////////////
    /**
     * @brief setMaxTTL Set Max TTL for parsing data
     * @param value max times parseData should be called
     */
    void setMaxTTL(const size_t &value);

    void setStreamableObject(StreamableObject *value);

protected:
    //////////////////////////////////////////
    // Virtual functions to initialize the protocol.
    virtual bool initProtocol() = 0;
    virtual void endProtocol() = 0;
    virtual bool changeToNextParser() = 0;

    StreamableObject * streamableObject;

    void initSubParser(SubStreamParser * subparser);
    SubStreamParser * currentParser;
    size_t maxTTL;
    bool initialized;
    bool clientMode;

private:
    /**
     * @brief parseData parse data into current parser. This calls recursively
     *                  until all data is filled.
     * @param buf data to be parsed
     * @param count data size to be parsed
     * @param ttl Time To Live Counter.
     * @return -1 if error, and n>0 : n length of data processed by parser, which should be equal to count.
     */
    std::pair<bool,uint64_t> parseData(const void * buf, size_t count, size_t *ttl, bool * finished);

};

#endif // INTERNALMAINPARSER_H
