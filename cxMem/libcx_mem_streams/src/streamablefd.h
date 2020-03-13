#ifndef STREAMABLEFD_H
#define STREAMABLEFD_H

#include "streamableobject.h"
#include <unistd.h>

class StreamableFD : public StreamableObject
{
public:
    StreamableFD(int _rd_fd = STDIN_FILENO, int _wr_fd = STDOUT_FILENO);
    /**
     * Retrieve Stream to another StreamableObject.
     * @param objDst pointer to the destination object.
     * @return false if failed, true otherwise.
     */
    virtual bool streamTo(StreamableObject * out, WRStatus & wrStatUpd) override;

    virtual WRStatus write(const void * buf, const size_t &count, WRStatus & wrStatUpd) override;


private:
    int rd_fd,wr_fd;

};

#endif // STREAMABLEFD_H
