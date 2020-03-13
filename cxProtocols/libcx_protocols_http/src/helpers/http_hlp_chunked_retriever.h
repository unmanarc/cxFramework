#ifndef HTTP_HLP_CHUNKED_RETRIEVER_H
#define HTTP_HLP_CHUNKED_RETRIEVER_H

#include <cx_mem_streams/streamableobject.h>

class HTTP_HLP_Chunked_Retriever : public StreamableObject
{
public:
    HTTP_HLP_Chunked_Retriever( StreamableObject * dst );
    ~HTTP_HLP_Chunked_Retriever( );


    bool streamTo(StreamableObject * out, WRStatus & wrsStat) override;
    WRStatus write(const void * buf, const size_t &count, WRStatus & wrStatUpd) override;


    bool endBuffer();

private:
    StreamableObject * dst;
    uint64_t pos;

};

#endif // HTTP_HLP_CHUNKED_RETRIEVER_H
