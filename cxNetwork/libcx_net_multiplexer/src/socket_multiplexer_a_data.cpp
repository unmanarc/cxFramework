#include "socket_multiplexer.h"

bool Socket_Multiplexer::multiplexedSocket_sendLineData(const sLineID &lineId, void *data, const uint16_t &datalen)
{
    if (noSendData) return false;
    std::unique_lock<std::timed_mutex> lock(mtLock_multiplexedSocket);

    if (!multiplexedSocket->writeU8(MPLX_LINE_DATA))
    {
        return false;
    }
    if (!sendOnMultiplexedSocket_LineID(lineId.remoteLineId))
    {
        return false;
    }
    if (!sendOnMultiplexedSocket_LineID(lineId.localLineId))
    {
        return false;
    }
    if (!multiplexedSocket->writeU16(datalen))
    {
        return false;
    }

    if (datalen)
    {
        if (!multiplexedSocket->writeBlock(data,datalen))
        {
            return false;
        }
    }

    return true;
}

bool Socket_Multiplexer::multiplexedSocket_sendTermination(const sLineID &lineId)
{
    return multiplexedSocket_sendLineData(lineId, nullptr, 0);
}

bool Socket_Multiplexer::multiplexedSocket_sendReadenBytes(const sLineID &lineId, const uint16_t &freedSize)
{
    if (noSendData) return false;
    std::unique_lock<std::timed_mutex> lock(mtLock_multiplexedSocket);

    if (!multiplexedSocket->writeU8(MPLX_LINE_BYTESREADEN))
    {
        return false;
    }
    if (!sendOnMultiplexedSocket_LineID(lineId.remoteLineId))
    {
        return false;
    }
    if (!sendOnMultiplexedSocket_LineID(lineId.localLineId))
    {
        return false;
    }
    if (!multiplexedSocket->writeU16(freedSize))
    {
        return false;
    }

    return true;
}

bool Socket_Multiplexer::processMultiplexedSocketCommand_Line_Data()
{
    bool readen;

    sLineID lineId;

    lineId.localLineId = recvFromMultiplexedSocket_LineID(&readen);
    lineId.remoteLineId = recvFromMultiplexedSocket_LineID(&readen);

    uint16_t sizeToRead = multiplexedSocket->readU16(&readen);
    if (!readen) return false;
    if (sizeToRead)
    {
        uint32_t recvBytes;
        if (!(multiplexedSocket->readBlock(readData,sizeToRead,&recvBytes) && recvBytes))
            return false;
    }
    std::shared_ptr<Socket_Multiplexed_Line> chSock = findLine(lineId.localLineId);
    if (!chSock->isValidLine())
    {
        // return back an order to close the connection:
        if (sizeToRead != 0)
            multiplexedSocket_sendTermination(lineId);
        else
        {
            // Action not required. line already out.
        }
    }
    else
    {
        // now readData have the data to be injected into the line.
        return chSock->addBufferElement(readData,sizeToRead);
    }

    return true;
}

bool Socket_Multiplexer::processMultiplexedSocketCommand_Line_UpdateReadenBytes()
{
    bool readen;

    sLineID lineId;

    lineId.localLineId = recvFromMultiplexedSocket_LineID(&readen);
    lineId.remoteLineId = recvFromMultiplexedSocket_LineID(&readen);

    uint16_t processedBytes = multiplexedSocket->readU16(&readen);
    if (!readen) return false;

    // Load the connection...
    std::shared_ptr<Socket_Multiplexed_Line> chSock = findLine(lineId.localLineId);

    if (!chSock->isValidLine())
    {
        multiplexedSocket_sendTermination(lineId);
        return true;
    }
    else
    {
        if (!chSock->addProcessedBytes(processedBytes)) return false;
    }


    return true;
}
