#ifndef SOCKET_BRIDGE
#define SOCKET_BRIDGE

#include "socket_base_stream.h"
#include "bridge_thread.h"

#include <stdint.h>

#include <atomic>
#include <thread>

/**
 * @brief The Socket_Bridge class connect two pipe sockets.
 */
class Socket_Stream_Bridge
{
public:
    /**
     * @brief Socket_Bridge constructor.
     */
    Socket_Stream_Bridge();
    /**
     * @brief Socket_Bridge destructor.
     */
    ~Socket_Stream_Bridge();
    /**
     * @brief start, begin the communication between peers in threaded mode.
     * @param autoDelete true (default) if going to delete the whole pipe when finish.
     * @return true if initialized, false if not.
     */
    bool start(bool _autoDeleteStreamPipeOnExit = true, bool detach = true);
    /**
     * @brief wait will block-wait until thread finishes
     * @return -1 failed, 0: socket 0 closed the connection, 1: socket 1 closed the connection.
     */
    int wait();

    /**
     * @brief process, begin the communication between peers blocking until it ends.
     * @return -1 failed, 0: socket 0 closed the connection, 1: socket 1 closed the connection.
     */
    int process();
    /**
     * @brief processPeer, begin the communication between peer i to the next peer.
     * @param i peer number (0 or 1)
     * @return true if transmitted and closed, false if failed.
     */
    bool processPeer(unsigned char i);
    /**
     * @brief SetPeer Set Stream Socket Peer (0 or 1)
     * @param i peer number: 0 or 1.
     * @param s peer established socket.
     * @return true if peer setted successfully.
     */
    bool setPeer(unsigned char i, Socket_Base_Stream * s);
    /**
     * @brief GetPeer Get the Pipe Peers
     * @param i peer number (0 or 1)
     * @return Stream Socket Peer.
     */
    Socket_Base_Stream * getPeer(unsigned char i);
    /**
     * @brief setAutoDelete Auto Delete the pipe object when finish threaded job.
     * @param value true for autodelete (default), false for not.
     */
    void setAutoDeleteStreamPipeOnThreadExit(bool value = true);
    /**
     * @brief shutdownRemotePeer set to shutdown both sockets peer on finish.
     * @param value true for close the remote peer (default), false for not.
     */
    void setToShutdownRemotePeer(bool value = true);
    /**
     * @brief closeRemotePeer set to close both sockets peer on finish.
     * @param value true for close the remote peer (default), false for not.
     */
    void setToCloseRemotePeer(bool value = true);
    /**
     * @brief getSentBytes Get bytes transmitted from peer 0 to peer 1.
     * @return bytes transmitted.
     */
    uint64_t getSentBytes() const;
    /**
     * @brief getRecvBytes Get bytes  transmitted from peer 1 to peer 0.
     * @return bytes transmitted.
     */
    uint64_t getRecvBytes() const;
    /**
     * @brief isAutoDeleteStreamPipeOnThreadExit Get if this class autodeletes when pipe is over.
     * @return true if autodelete is on.
     */
    bool isAutoDeleteStreamPipeOnThreadExit() const;
    /**
     * @brief isAutoDeleteSocketsOnExit Get if pipe endpoint sockets are going to be deleted when this class is destroyed.
     * @return true if it's going to be deleted.
     */
    bool isAutoDeleteSocketsOnExit() const;
    /**
     * @brief setAutoDeleteSocketsOnExit Set if pipe endpoint sockets are going to be deleted when this class is destroyed.
     * @param value true if you want sockets to be deleted on exit.
     */
    void setAutoDeleteSocketsOnExit(bool value);
    /**
     * @brief setCustomPipeProcessor Set custom pipe processor.
     * @param value pipe processor.
     */
    void setCustomPipeProcessor(Bridge_Thread *value, bool deleteOnExit = false);

private:
    Bridge_Thread *bridgeThreadPrc;
    Socket_Base_Stream * socket_peers[2];
    std::atomic<uint64_t> sentBytes,recvBytes;
    std::atomic<int> finishingPeer;
    std::atomic<bool> shutdownRemotePeerOnFinish;
    std::atomic<bool> closeRemotePeerOnFinish;

    bool autoDeleteStreamPipeOnExit;
    bool autoDeleteSocketsOnExit;
    bool autoDeleteCustomPipeOnClose;

    std::thread pipeThreadP;
};

#endif // SOCKET_BRIDGE
