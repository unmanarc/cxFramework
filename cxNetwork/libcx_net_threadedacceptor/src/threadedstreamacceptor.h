#ifndef VSTREAMACCEPTOR_H
#define VSTREAMACCEPTOR_H

#include <list>
#include <map>
#include <thread>
#include <condition_variable>

#include "streamthread.h"
#include <cx_net_sockets/socket_base_stream.h>

/**
 * @brief The ThreadedStreamAcceptor class Accept streams on thread from a listening socket.
 */
class ThreadedStreamAcceptor
{
public:
    /**
     * Constructor
     */
    ThreadedStreamAcceptor();
    /**
     * Destructor
     * WARN: when you finalize this class, the listening socket is closed. please open another one (don't reuse it)
     */
    ~ThreadedStreamAcceptor();
    /**
     * @brief startThreaded Start accepting connections in a new thread (will wait for finalization in destructor)
     */
    void startThreaded();
    /**
     * @brief startBlocking Start Accepting Connections in your own thread.
     * @return
     */
    bool startBlocking();
    /**
     * @brief stop Stop Acceptor
     */
    void stop();
    /**
     * Set callback when connection is fully established (if the callback returns false, connection socket won't be automatically closed/deleted)
     */
    void setCallbackOnConnect(bool (*_callbackOnConnect)(void *, Socket_Base_Stream *, const char *), void *obj);
    /**
     * Set callback when protocol initialization failed (like bad X.509 on TLS) (if the callback returns false, connection socket won't be automatically closed/deleted)
     */
    void setCallbackOnInitFail(bool (*_callbackOnInitFailed)(void *, Socket_Base_Stream *, const char *), void *obj);
    /**
     * Set callback when timed out (max concurrent clients reached and timed out) (this callback is called from acceptor thread, you should use it very quick)
     */
    void setCallbackOnTimedOut(void (*_callbackOnTimedOut)(void *, Socket_Base_Stream *, const char *), void *obj);
    /**
     * Set callback when maximum connections per IP reached (this callback is called from acceptor thread, you should use it very quick)
     */
    void setCallbackOnMaxConnectionsPerIP(void (*_callbackOnMaxConnectionsPerIP)(void *, Socket_Base_Stream *, const char *), void *obj);

    /**
     * Set the socket that will be used to accept new clients.
     * WARNING: acceptorSocket will be deleted when this class finishes.
     */
    void setAcceptorSocket(Socket_Base_Stream *acceptorSocket);
    /**
     * Do accept on the acceptor socket.
     * @return true if we can still accept a new connection
     */
    bool acceptClient();
    /**
     * Finalize/Catch the client thread element (when it finishes).
     */
    bool finalizeThreadElement(StreamThread * x);
    /**
     * @brief getMaxConcurrentClients Get maximum number of concurrent client threads are accepted
     * @return maximum current clients accepted
     */
    uint32_t getMaxConcurrentClients();
    /**
     * @brief setMaxConcurrentClients Set maximum number of client threads accepted at the same time. Beware that too many concurrent threads could lead to an unhandled exception an program fault (check your ulimits).
     * @param value maximum current clients accepted
     */
    void setMaxConcurrentClients(const uint32_t &value);
    /**
     * @brief getMaxWaitMSTime Get maximum time to wait if maximum concurrent limit reached
     * @return time in milliseconds
     */
    uint32_t getMaxWaitMSTime();
    /**
     * @brief setMaxWaitMSTime Set maximum time to wait if maximum concurrent limit reached
     * @param value time in milliseconds
     */
    void setMaxWaitMSTime(const uint32_t &value);
    /**
     * @brief getMaxConnectionsPerIP Get maximum concurrent connections per client IP
     * @return maximum number of connections allowed
     */
    uint32_t getMaxConnectionsPerIP();
    /**
     * @brief setMaxConnectionsPerIP Set maximum concurrent connections per client IP
     * @param value maximum number of connections allowed
     */
    void setMaxConnectionsPerIP(const uint32_t &value);

private:

    bool processClient(Socket_Base_Stream * clientSocket, StreamThread * clientThread);

    uint32_t incrementIPUsage(const std::string & ipAddr);
    void decrementIPUsage(const std::string & ipAddr);

    void init();

    bool initialized, finalized;
    Socket_Base_Stream * acceptorSocket;
    std::list<StreamThread *> threadList;
    std::map<std::string, uint32_t> connectionsPerIP;

    // Callbacks:
    bool (*callbackOnConnect)(void *,Socket_Base_Stream *, const char *);
    bool (*callbackOnInitFail)(void *,Socket_Base_Stream *, const char *);
    void (*callbackOnTimedOut)(void *,Socket_Base_Stream *, const char *);
    void (*callbackOnMaxConnectionsPerIP)(void *,Socket_Base_Stream *, const char *);

    void *objOnConnect, *objOnInitFail, *objOnTimedOut, *objOnMaxConnectionsPerIP;

    // thread objects:
    std::thread acceptorThread;

    //
    uint32_t maxConcurrentClients, maxWaitMSTime, maxConnectionsPerIP;
    std::mutex mutex_clients;
    std::condition_variable cond_clients_empty, cond_clients_notfull;
};

#endif // VSTREAMACCEPTOR_H
