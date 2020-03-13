#include "http_hlp_chunked_retriever.h"
#include <stdio.h>
#include <string.h>

HTTP_HLP_Chunked_Retriever::HTTP_HLP_Chunked_Retriever(StreamableObject *dst)
{
    this->dst = dst;
    pos = 0;
}

HTTP_HLP_Chunked_Retriever::~HTTP_HLP_Chunked_Retriever()
{
    endBuffer();
}

bool HTTP_HLP_Chunked_Retriever::streamTo(StreamableObject *out, WRStatus &wrsStat)
{
    return false;
}

WRStatus HTTP_HLP_Chunked_Retriever::write(const void *buf, const size_t &count, WRStatus &wrStat)
{
    WRStatus cur;
    char strhex[32];

    if (count+64<count) { cur.succeed=wrStat.succeed=setFailedWriteState(); return cur; }
    snprintf(strhex,32, pos == 0?"%X\r\n":"\r\n%X\r\n", (unsigned int)count);

    if (!(cur+=dst->writeString(strhex,wrStat)).succeed) { cur.succeed=wrStat.succeed=setFailedWriteState(); return cur; }
    if (!(cur+=dst->writeFullStream(buf,count,wrStat)).succeed) { cur.succeed=wrStat.succeed=setFailedWriteState(); return cur; }

    pos+=count;

    return cur;
}

bool HTTP_HLP_Chunked_Retriever::endBuffer()
{
    WRStatus cur;
    return (cur=dst->writeString(pos == 0? "0\r\n\r\n" : "\r\n0\r\n\r\n",cur)).succeed;
}
