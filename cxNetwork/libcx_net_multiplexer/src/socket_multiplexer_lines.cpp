#include "socket_multiplexer_lines.h"
#include "socket_multiplexer.h"

Socket_Multiplexer_Lines::Socket_Multiplexer_Lines()
{
    bNoNewConnections = true;
    std::random_device rd;
    gen.seed(rd());

    // 65535 searches. (media: 2 searches means that 2.000.000.000 slots are in use)
    maxMissedSearches = 0xFF;
}

Socket_Multiplexer_Lines::~Socket_Multiplexer_Lines()
{
}

// for outgoing connection ( syn/ack equiv should set the remote number )
std::shared_ptr<Socket_Multiplexed_Line> Socket_Multiplexer_Lines::registerLine()
{
    Lock_Mutex_RW lock(rwLock_MultiplexedLines);

    std::shared_ptr<Socket_Multiplexed_Line> r = std::make_shared<Socket_Multiplexed_Line>();
    if (bNoNewConnections)
    {
        return r; // no more slots available.
    }
    if (multiplexedLinesMap.empty())
        mtLock_OnEmptyLines.lock();

    // Look out for your line id.
    LineID i=findLocalAvailableSlot();
    if (i != NULL_LINE)
    {
        r->setLineLocalID(i);
        multiplexedLinesMap[i] = r;
    }
    else
    {
        // prevent double locking.
        mtLock_OnEmptyLines.unlock();
    }
    return r;
}

std::shared_ptr<Socket_Multiplexed_Line> Socket_Multiplexer_Lines::findLine(LineID localLineId)
{
    Lock_Mutex_RD lock(rwLock_MultiplexedLines);

    std::shared_ptr<Socket_Multiplexed_Line> x;
    if (multiplexedLinesMap.find(localLineId) == multiplexedLinesMap.end())
        x = std::make_shared<Socket_Multiplexed_Line>();
    else
        x = multiplexedLinesMap[localLineId];

    return x;
}

void Socket_Multiplexer_Lines::removeLine(LineID lineId)
{
    Lock_Mutex_RW lock(rwLock_MultiplexedLines);

    if (multiplexedLinesMap.find(lineId) != multiplexedLinesMap.end())
    {
        multiplexedLinesMap.erase(lineId);
        // TODO: prevent double unlock. / pass to signal
        if (multiplexedLinesMap.empty())
            mtLock_OnEmptyLines.unlock();
    }
    else
    {
        //DEBUG
    }
}

void Socket_Multiplexer_Lines::stopAndRemoveLine(std::shared_ptr<Socket_Multiplexed_Line> muxLineSocket)
{
    removeLine(muxLineSocket->getLineID().localLineId);
    muxLineSocket->finalizeProcessor();
}

LineID Socket_Multiplexer_Lines::findLocalAvailableSlot()
{
    for (uint32_t i=0;i<maxMissedSearches;i++)
    {
        LineID ntry = dis(gen);
        if (ntry != NULL_LINE)
        {
            if (multiplexedLinesMap.find(ntry)==multiplexedLinesMap.end())
                return ntry;
        }
    }
    return NULL_LINE;
}

uint32_t Socket_Multiplexer_Lines::getMaxMissedSearches() const
{
    return maxMissedSearches;
}

void Socket_Multiplexer_Lines::closeAndWaitForEveryLine()
{
    // Prevent new connections...
     // TODO: lock rwLock_MultiplexedLines
    //bNoNewConnections = true;

    // Send close data.
    if (true)
    {
        Lock_Mutex_RD lock(rwLock_MultiplexedLines);
        for ( std::pair<LineID,std::shared_ptr<Socket_Multiplexed_Line>> cs : multiplexedLinesMap )
        {
            cs.second->_lshutdown(); // the loosy way.

            //if connections are overwhelmed, then the nullptr/0 won't reach and the connection won't end...
            //cs.second->writeDataFromBC(nullptr,0);
            cs.second->resetRemoteAvailableBytes();
        }
    }

    // Wait...
    if (true)
    {
        std::unique_lock<std::mutex> lock(mtLock_OnEmptyLines);
    }
}

void Socket_Multiplexer_Lines::setMaxMissedSearches(const uint32_t &value)
{
    maxMissedSearches = value;
}

void Socket_Multiplexer_Lines::unlockNewConnections()
{
    Lock_Mutex_RW lock(rwLock_MultiplexedLines);
    bNoNewConnections = false;
}

void Socket_Multiplexer_Lines::preventNewConnections()
{
    Lock_Mutex_RW lock(rwLock_MultiplexedLines);
    bNoNewConnections = true;
}

