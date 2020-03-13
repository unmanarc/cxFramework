#ifndef SOCKET_MULTIPLEXER_LINES_H
#define SOCKET_MULTIPLEXER_LINES_H

#include <map>
#include <random>
#include <atomic>
#include <memory>
#include "socket_multiplexed_line.h"

class Socket_Multiplexer_Lines
{
public:
    Socket_Multiplexer_Lines();
    virtual ~Socket_Multiplexer_Lines();

    uint32_t getMaxMissedSearches() const;
    void setMaxMissedSearches(const uint32_t &value);

protected:
    void unlockNewConnections();
    void preventNewConnections();
    void closeAndWaitForEveryLine();

    std::shared_ptr<Socket_Multiplexed_Line> registerLine();
    std::shared_ptr<Socket_Multiplexed_Line> findLine(LineID localLineId);

    void stopAndRemoveLine(std::shared_ptr<Socket_Multiplexed_Line> muxLineSocket);

private:
    void removeLine(LineID lineId);
    LineID findLocalAvailableSlot();

    // Lines list.
    std::map<LineID, std::shared_ptr<Socket_Multiplexed_Line>> multiplexedLinesMap;

    // Mutex Locks
    Mutex_RW rwLock_MultiplexedLines;
    Mutex mtLock_OnEmptyLines;

    // Random distribution
    std::mt19937_64 gen;
    std::uniform_int_distribution<LineID> dis;

    // Search algorithm
    std::atomic<uint32_t> maxMissedSearches;

    // Stop algorithm
    bool bNoNewConnections;
};

#endif // SOCKET_MULTIPLEXER_LINES_H
