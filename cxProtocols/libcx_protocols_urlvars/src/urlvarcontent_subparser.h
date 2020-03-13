#ifndef HTTP_SUB_URLVARPARSER_H
#define HTTP_SUB_URLVARPARSER_H

#include <cx_mem_streamparser/substreamparser.h>

class URLVarContent_SubParser : public SubStreamParser
{
public:
    URLVarContent_SubParser();
    ~URLVarContent_SubParser() override;

    bool stream(WRStatus &) override;

    void setVarType(bool varName = true);
    void setMaxObjectSize(const uint32_t &size);
    BinaryContainer_Chunks *flushRetrievedContentAsBC();
    std::string flushRetrievedContentAsString();


    BinaryContainer_Chunks *getPData();

protected:
    ParseStatus parse() override;
    BinaryContainer_Chunks * pData;

};

#endif // HTTP_SUB_URLVARPARSER_H
