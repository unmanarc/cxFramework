#ifndef SOCKET_MULTIPLEXER_A_STRUCT_LINEID_H
#define SOCKET_MULTIPLEXER_A_STRUCT_LINEID_H

#include "vars.h"
#include <limits>

#define NULL_LINE std::numeric_limits<LineID>::max()

struct sLineID
{
    sLineID()
    {
        localLineId = NULL_LINE;
        remoteLineId = NULL_LINE;
    }
    LineID localLineId, remoteLineId;
};

#endif // SOCKET_MULTIPLEXER_A_STRUCT_LINEID_H
