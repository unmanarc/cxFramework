#include "linerecv_subparser.h"

LineRecv_SubParser::LineRecv_SubParser()
{
    setParseMode(PARSE_MODE_MULTIDELIMITER);
    setParseMultiDelimiter({"\x0a", "\x0d"});
    setMaxObjectSize(65536);
}

LineRecv_SubParser::~LineRecv_SubParser()
{
}

void LineRecv_SubParser::setMaxObjectSize(const uint32_t &size)
{
    // TODO: test max limits...
    setParseDataTargetSize(size);
}

bool LineRecv_SubParser::stream(WRStatus &)
{
    // NOT IMPLEMENTED.
    return false;
}

std::string LineRecv_SubParser::getParsedString() const
{
    return parsedString;
}

ParseStatus LineRecv_SubParser::parse()
{
    parsedString = getParsedData()->toString();
    return PARSE_STAT_GOTO_NEXT_SUBPARSER;
}
