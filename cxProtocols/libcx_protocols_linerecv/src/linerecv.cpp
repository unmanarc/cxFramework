#include "linerecv.h"

LineRecv::LineRecv(StreamableObject *sobject) : StreamParser(sobject,false)
{
    initialized = initProtocol();
    currentParser = (SubStreamParser *)(&subParser);
}

void LineRecv::setMaxLineSize(const uint32_t &maxLineSize)
{
    subParser.setMaxObjectSize(maxLineSize);
}

bool LineRecv::initProtocol()
{
    return true;
}

bool LineRecv::changeToNextParser()
{
    if (!processParsedLine( subParser.getParsedString() ))
    {
        currentParser = nullptr;
        return false;
    }
    return true;
}


