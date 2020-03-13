#ifndef MIME_SUB_ENDPBOUNDARY_H
#define MIME_SUB_ENDPBOUNDARY_H

#include <cx_mem_streamparser/substreamparser.h>

#define ENDP_STAT_UNINITIALIZED -1
#define ENDP_STAT_CONTINUE 0
#define ENDP_STAT_END 1
#define ENDP_STAT_ERROR 2

class MIME_Sub_EndPBoundary : public SubStreamParser
{
public:
    MIME_Sub_EndPBoundary();
    bool stream(WRStatus &wrStat) override;
    int getStatus() const;

protected:
    ParseStatus parse() override;

private:
    int status;
};

#endif // MIME_SUB_ENDPBOUNDARY_H
