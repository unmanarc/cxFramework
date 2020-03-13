#include "substreamparser.h"
#include <cx_mem_containers/binarycontainer_ref.h>

SubStreamParser::SubStreamParser()
{
    upStream = nullptr;
    streamEnded = false;
    setParseStatus(PARSE_STAT_GET_MORE_DATA);
    parseMode = PARSE_MODE_DELIMITER;
    parseDelimiter = "\r\n";
    leftToParse = std::numeric_limits<uint64_t>::max();
}

SubStreamParser::~SubStreamParser()
{
}

void SubStreamParser::initElemParser(StreamableObject *upstreamObj, bool clientMode)
{
    this->upStream = upstreamObj;
    this->clientMode = clientMode;
}

std::pair<bool,uint64_t> SubStreamParser::writeIntoParser(const void *buf, size_t count)
{
    if (!count) streamEnded = true;
    switch (parseMode)
    {
    case PARSE_MODE_DELIMITER:
        return parseByDelimiter(buf,count);
    case PARSE_MODE_SIZE:
        return parseBySize(buf,count);
    case PARSE_MODE_VALIDATOR:
        return parseByValidator(buf,count);
    case PARSE_MODE_DIRECT_DELIMITER:
        return parseDirectDelimiter(buf,count);
    case PARSE_MODE_CONNECTION_END:
        return parseByConnectionEnd(buf,count);
    case PARSE_MODE_MULTIDELIMITER:
        return parseByMultiDelimiter(buf,count);
    case PARSE_MODE_DIRECT:
        return parseDirect(buf,count);
    default:
        break;
    }
    return std::make_pair(false,(uint64_t)0);
}

size_t SubStreamParser::ParseValidator(BinaryContainer_Base &bc)
{
    return std::numeric_limits<size_t>::max();
}

ParseStatus SubStreamParser::getParseStatus() const
{
    return parseStatus;
}

void SubStreamParser::setParseStatus(const ParseStatus &value)
{
    parseStatus = value;
}

void SubStreamParser::setParseDelimiter(const std::string &value)
{
    parseDelimiter = value;
}

BinaryContainer_Base *SubStreamParser::getParsedData()
{
    return &parsedBuffer;
}

std::pair<bool,uint64_t> SubStreamParser::parseByMultiDelimiter(const void *buf, size_t count)
{
    std::pair<bool,uint64_t> needlePos, bytesAppended = std::make_pair(true,0);
    uint64_t prevSize = unparsedBuffer.size(), bytesToDisplace = 0;
    if (!count)
    {
        streamEnded = true;
       /* if (!unparsedBuffer.size())
        {
            return 0;
        }*/
    }

    if (count && ((bytesAppended=unparsedBuffer.append(buf,count)).first==false || bytesAppended.second==0))
    {
        // size exceeded. don't continue with the streamparser, error.
        unparsedBuffer.clear();
        return std::make_pair(false,(uint64_t)0);
    }

    bytesToDisplace = bytesAppended.second;
    parsedBuffer.reference(&unparsedBuffer);

    if ( (needlePos = unparsedBuffer.find(parseMultiDelimiter, delimiterFound)).first!=false )
    {
        //std::cout << "DELIMITER FOUND AT ->" << needlePos.second << std::endl << std::flush;

        // needle found.
        parsedBuffer.reference(&unparsedBuffer,0,needlePos.second);

#ifdef DEBUG_PARSER
        printf("Parsing by multidelimiter: %s\n", postParsedBuffer.toString().c_str()); fflush(stdout);
#endif

        setParseStatus(parse());
        unparsedBuffer.clear();

        // Bytes to displace:
        bytesToDisplace = needlePos.second+delimiterFound.size()-prevSize;
    }
    else if (streamEnded)
    {
        parsedBuffer.reference(&unparsedBuffer);
        setParseStatus(parse());
        unparsedBuffer.clear();
    }
    else
    {
        //std::cout << "DELIMITER NOT FOUND HERE." << std::endl << std::flush;
    }

    return std::make_pair(true,bytesToDisplace);
}

std::pair<bool,uint64_t> SubStreamParser::parseByDelimiter(const void *buf, size_t count)
{
    std::pair<bool,uint64_t> needlePos, bytesAppended = std::make_pair(true,0);

    uint64_t prevSize = unparsedBuffer.size(), bytesToDisplace = 0;
    if (!count)
    {
        streamEnded = true;
    }

    if (count && ((bytesAppended=unparsedBuffer.append(buf,count)).second==0 || bytesAppended.first==false))
    {
        // size exceeded. don't continue with the streamparser, error.
        unparsedBuffer.clear();
        return std::make_pair(false,(uint64_t)0);
    }

    bytesToDisplace = bytesAppended.second;
    parsedBuffer.reference(&unparsedBuffer);

    if ( (needlePos = unparsedBuffer.find(parseDelimiter.c_str(),parseDelimiter.size())).first!=false )
    {
        // needle found.
        parsedBuffer.reference(&unparsedBuffer,0,needlePos.second);

#ifdef DEBUG_PARSER
        printf("Parsing by delimiter: %s\n", postParsedBuffer.toString().c_str()); fflush(stdout);
#endif

        setParseStatus(parse());
        unparsedBuffer.clear();

        // Bytes to displace:
        bytesToDisplace = (needlePos.second-prevSize)+parseDelimiter.size();
    }
    else if (streamEnded)
    {
        parsedBuffer.reference(&unparsedBuffer);
        setParseStatus(parse());
        unparsedBuffer.clear();
    }

    return std::make_pair(true,bytesToDisplace);
}

std::pair<bool,uint64_t> SubStreamParser::parseBySize(const void *buf, size_t count)
{
    if (!count)
    {
        // Abort current data.
        streamEnded = true;
        setParseStatus(PARSE_STAT_GOTO_NEXT_SUBPARSER);
        unparsedBuffer.clear(); // Destroy the container data.
        return std::make_pair(true,0);
    }

    uint64_t bytesToAppend = count>leftToParse?leftToParse : count;

    std::pair<bool,uint64_t> appendedBytes = unparsedBuffer.append(buf,bytesToAppend);

    if (!appendedBytes.first)
        return appendedBytes;

    if (appendedBytes.second!=bytesToAppend)
        return std::make_pair(false,appendedBytes.second);

    leftToParse-=bytesToAppend;

    if (leftToParse==0)
    {
        parsedBuffer.reference(&unparsedBuffer);

#ifdef DEBUG_PARSER
        printf("Parsing by size: %s\n", postParsedBuffer.toString().c_str()); fflush(stdout);
#endif
        setParseStatus(parse());
        unparsedBuffer.clear(); // Destroy the container data.
    }

    return std::make_pair(true,bytesToAppend);
}

std::pair<bool,uint64_t> SubStreamParser::parseByValidator(const void *buf, size_t count)
{
    /*BinaryContainer_MEM bc;

    size_t origSize = postParsedBuffer.getContainerSize();

    bc.setMaxSize(postParsedBuffer.getMaxSize());
    postParsedBuffer.appendTo(bc);
    if (bc.append(buf,count)!=count) return -1;
    size_t validatedBytes = ParseValidator(bc);
    if (validatedBytes == -1)
    {
        // NOT FOUND:
        if (postParsedBuffer.append(buf,count)!=count) return -1;
        return count;
    }

    size_t bytesLeft = bc.getContainerSize()-validatedBytes;
    */
    // TODO:
    return std::make_pair(false,(uint64_t)0);
}

std::pair<bool,uint64_t> SubStreamParser::parseByConnectionEnd(const void *buf, size_t count)
{
    std::pair<bool,uint64_t> appendedBytes = unparsedBuffer.append(buf,count);

    if (!appendedBytes.first) return std::make_pair(false,(uint64_t)0);
    if (appendedBytes.second!=count) return std::make_pair(false,appendedBytes.second); // max reached.

    if (!count)
    {
        parsedBuffer.reference(&unparsedBuffer);
#ifdef DEBUG_PARSER
        printf("Parsing by connection end: %s\n", postParsedBuffer.toString().c_str()); fflush(stdout);
#endif
        setParseStatus(parse()); // analyze on connection end.
        parsedBuffer.clear();
        return std::make_pair(true,0);
    }
    return std::make_pair(true,count);
}

std::pair<bool,uint64_t> SubStreamParser::parseDirect(const void *buf, size_t count)
{
    std::pair<bool,uint64_t> copiedDirect;
    size_t bytesToAppend = (leftToParse == std::numeric_limits<uint64_t>::max() || leftToParse>count) ? count : leftToParse;

    // TODO: termination?

    // Bad copy...
    if ((copiedDirect = unparsedBuffer.append(buf, bytesToAppend)).first == false)
        return std::make_pair(false,(uint64_t)0);

    // now there is less data left.
    if (leftToParse != std::numeric_limits<uint64_t>::max())
        leftToParse-=copiedDirect.second;

    // Parse it...
    parsedBuffer.reference(&unparsedBuffer);

#ifdef DEBUG_PARSER
    printf("Parsing direct (%llu, until size %llu): %s\n", postParsedBuffer.size(), leftToParse, postParsedBuffer.toString().c_str()); fflush(stdout);
#endif

    setParseStatus(parse());
    unparsedBuffer.clear(); // Reset the container data for the next element.

    // Reset the left to parse counter.
    if (leftToParse==0)
        leftToParse = unparsedBuffer.getMaxSize();

    return copiedDirect;
}


std::pair<bool,uint64_t> SubStreamParser::parseDirectDelimiter(const void *buf, size_t count)
{
    std::pair<bool,uint64_t> delimPos;
    std::pair<bool,uint64_t> copiedDirect;

    uint64_t prevSize=0;
    delimiterFound = "";

    // TODO: termination?

    if (count && leftToParse==0)
        return std::make_pair(false,(uint64_t)0); // Can't handle more data...
    else if (leftToParse != std::numeric_limits<uint64_t>::max() && count>leftToParse)
        count = leftToParse; // handle only left to parse

    // Get the previous size.
    prevSize = unparsedBuffer.size();
    // Append the current data
    if ((copiedDirect = unparsedBuffer.append(buf,count)).first==false)
        return std::make_pair(false,(uint64_t)0);


    // Find the delimiter pos.
    if ((delimPos = unparsedBuffer.find(parseDelimiter.c_str(),parseDelimiter.size())).first==false)
    {
        // Not found, evaluate how much data I can give here...
        uint64_t bytesOfPossibleDelim = getLastBytesInCommon(parseDelimiter);
        switch (bytesOfPossibleDelim)
        {
        case std::numeric_limits<uint64_t>::max():
            // delimiter not found at all, parse ALL direct...
            parsedBuffer.reference(&unparsedBuffer);
            //printf("Parsing direct delimiter (%llu, until size %llu): ", postParsedBuffer.size(), leftToParse); postParsedBuffer.print(); printf("\n"); fflush(stdout);
            setParseStatus(parse());
            unparsedBuffer.clear(); // Reset the container data for the next element.
            break;
        case 0:
            // empty (should not occur)...
            break;
        default:
            // bytesOfPossibleDelim possibly belongs to the delimiter.
            // we are safe on unparsedBuffer.size()-bytesOfPossibleDelim
            parsedBuffer.reference(&unparsedBuffer, 0,unparsedBuffer.size()-bytesOfPossibleDelim);
            setParseStatus(parse());
            unparsedBuffer.clear(); // Reset the container data for the next element.
            break;
        }

        if (leftToParse!= std::numeric_limits<uint64_t>::max())
            leftToParse -= copiedDirect.second;

        return std::make_pair(true,parseDelimiter.size()-prevSize); // Move all data copied.
    }
    else
    {
        // DELIMITER Found...
        delimiterFound = parseDelimiter;

        if (delimPos.second==0)
        {
            unparsedBuffer.clear(); // found on 0... give nothing to give
            parsedBuffer.reference(&unparsedBuffer);
        }
        else parsedBuffer.reference(&unparsedBuffer,0,delimPos.second);

        setParseStatus(parse());
        unparsedBuffer.clear(); // Reset the container data for the next element.

        // reestablish the left to parse (delim found).
        if (leftToParse!=std::numeric_limits<uint64_t>::max()) leftToParse = unparsedBuffer.getMaxSize();

        return std::make_pair(true,(delimPos.second-prevSize)+parseDelimiter.size());
    }
}

uint64_t SubStreamParser::getLastBytesInCommon(const std::string &boundary)
{
    size_t maxBoundary = unparsedBuffer.size()>(boundary.size()-1)?(boundary.size()-1) : unparsedBuffer.size();
    for (size_t v=maxBoundary; v!=MAX_SIZE_T; v--)
    {
        BinaryContainer_Ref ref = referenceLastBytes(v);
        char * toCmp = ((char *)malloc(ref.size()));
        ref.copyOut(toCmp,ref.size());
        if (!memcmp(toCmp,boundary.c_str(),ref.size()))
        {
            free(toCmp);
            return v;
        }
        free(toCmp);
    }
    return std::numeric_limits<uint64_t>::max();
}

BinaryContainer_Ref SubStreamParser::referenceLastBytes(const size_t &bytes)
{
    BinaryContainer_Ref r;
    r.reference(&unparsedBuffer, unparsedBuffer.size()-bytes);
    return r;
}

std::string SubStreamParser::getDelimiterFound() const
{
    return delimiterFound;
}

void SubStreamParser::setParseMultiDelimiter(const std::list<std::string> &value)
{
    parseMultiDelimiter = value;
}

uint64_t SubStreamParser::getLeftToparse() const
{
    return leftToParse;
}

void SubStreamParser::setParseDataTargetSize(const uint64_t &value)
{
    leftToParse = value;
    unparsedBuffer.setMaxSize(value);
}

bool SubStreamParser::isStreamEnded() const
{
    return streamEnded;
}

void SubStreamParser::setParseMode(const ParseMode &value)
{
    if (value == PARSE_MODE_DIRECT) setParseDataTargetSize(std::numeric_limits<uint64_t>::max());
    parseMode = value;
}
