#ifndef HTTP1BASE_H
#define HTTP1BASE_H

#include <cx_mem_streamparser/streamparser.h>
#include <netinet/in.h>
#include "linerecv_subparser.h"

class LineRecv : public StreamParser
{
public:
    LineRecv(StreamableObject *sobject);
    virtual ~LineRecv()  override {}
    void setMaxLineSize(const uint32_t & maxLineSize);

protected:
    virtual bool processParsedLine(const std::string & line) =0;

    virtual bool initProtocol() override;
    virtual void endProtocol() override {}
    virtual bool changeToNextParser() override;
    LineRecv_SubParser subParser;

};

#endif // HTTP1BASE_H
