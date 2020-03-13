#ifndef LINERECV_SUBPARSER_H
#define LINERECV_SUBPARSER_H

#include <cx_mem_streamparser/substreamparser.h>

class LineRecv_SubParser : public SubStreamParser
{
public:
    LineRecv_SubParser();
    ~LineRecv_SubParser() override;

    void setMaxObjectSize(const uint32_t &size);
    bool stream(WRStatus &) override;

    std::string getParsedString() const;

protected:
    std::string parsedString;
    ParseStatus parse() override;

};


#endif // LINERECV_SUBPARSER_H
