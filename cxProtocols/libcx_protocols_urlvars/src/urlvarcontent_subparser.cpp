#include "urlvarcontent_subparser.h"

#include <cx_mem_streamencoders/streamdecoder_url.h>

URLVarContent_SubParser::URLVarContent_SubParser()
{
    setParseMode(PARSE_MODE_MULTIDELIMITER);
    setParseMultiDelimiter({"=","&"});
    setMaxObjectSize(4096);
    pData = new BinaryContainer_Chunks;
}

URLVarContent_SubParser::~URLVarContent_SubParser()
{
    if (pData) delete pData;
}

bool URLVarContent_SubParser::stream(WRStatus &)
{
    // NOT IMPLEMENTED.
    return false;
}

void URLVarContent_SubParser::setVarType(bool varName)
{
    if (varName)
        setParseMultiDelimiter({"=","&"}); // Parsing name...
    else
        setParseMultiDelimiter({"&"}); // Parsing value...
}

void URLVarContent_SubParser::setMaxObjectSize(const uint32_t &size)
{
    setParseDataTargetSize(size);
}

BinaryContainer_Chunks *URLVarContent_SubParser::flushRetrievedContentAsBC()
{
    BinaryContainer_Chunks * r = pData;
    pData = new BinaryContainer_Chunks;
    return r;
}

std::string URLVarContent_SubParser::flushRetrievedContentAsString()
{
    std::string r = pData->toString();
    delete pData;
    pData = new BinaryContainer_Chunks;
    return r;
}

ParseStatus URLVarContent_SubParser::parse()
{
    pData->clear();
    if (!getParsedData()->size()) return PARSE_STAT_GET_MORE_DATA;
    WRStatus cur;
    StreamDecoder_URL decUrl(pData);
    if (!(cur=getParsedData()->streamTo(&decUrl,cur)).succeed)
    {
        pData->clear();
    }
    return PARSE_STAT_GOTO_NEXT_SUBPARSER;
}

BinaryContainer_Chunks *URLVarContent_SubParser::getPData()
{
    return pData;
}
