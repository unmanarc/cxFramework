#ifndef MIME_SUB_ENDPOINT_H
#define MIME_SUB_ENDPOINT_H

#include <string>
#include <cx_mem_streamparser/substreamparser.h>

class MIME_Sub_FirstBoundary : public SubStreamParser
{
public:
    MIME_Sub_FirstBoundary();

    bool stream(WRStatus &wrStat) override;

    std::string getBoundary() const;
    void setBoundary(const std::string &value);

protected:
    ParseStatus parse() override;
private:
    std::string boundary;
};

#endif // MIME_SUB_ENDPOINT_H
