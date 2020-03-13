#ifndef NULLCONTAINER_H
#define NULLCONTAINER_H

#include <cx_mem_streams/streamableobject.h>

class NullContainer : public StreamableObject
{
public:
    NullContainer();

    bool streamTo(StreamableObject * out, WRStatus & wrsStat) override;
    WRStatus write(const void * buf, const size_t &count, WRStatus & wrStatUpd) override;

    uint64_t size() const override { return bytes; }

private:
    uint64_t bytes;
};

#endif // NULLCONTAINER_H
