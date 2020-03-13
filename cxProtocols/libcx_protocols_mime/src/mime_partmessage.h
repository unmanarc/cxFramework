#ifndef MIME_PARTMESSAGE_H
#define MIME_PARTMESSAGE_H

#include "mime_sub_content.h"
#include "mime_sub_header.h"

#include <cx_mem_streams/streamableobject.h>

class MIME_PartMessage
{
public:
    MIME_PartMessage();

    bool stream(WRStatus &wrStat);

    MIME_Sub_Content * getContent();
    MIME_Sub_Header * getHeader();

private:
    MIME_Sub_Content content;
    MIME_Sub_Header header;
};

#endif // MIME_PARTMESSAGE_H
