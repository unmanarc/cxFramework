#include "mime_sub_firstboundary.h"

MIME_Sub_FirstBoundary::MIME_Sub_FirstBoundary()
{
    setParseMode(PARSE_MODE_DELIMITER);
    setBoundary("XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX");
}

bool MIME_Sub_FirstBoundary::stream(WRStatus & wrStat)
{
    return true;
}

std::string MIME_Sub_FirstBoundary::getBoundary() const
{
    return boundary;
}

void MIME_Sub_FirstBoundary::setBoundary(const std::string &value)
{
    setParseDelimiter("--" + value + "\r\n");
    setParseDataTargetSize(value.size()+4);
    boundary = value;
}

ParseStatus MIME_Sub_FirstBoundary::parse()
{
    return PARSE_STAT_GOTO_NEXT_SUBPARSER;
}
