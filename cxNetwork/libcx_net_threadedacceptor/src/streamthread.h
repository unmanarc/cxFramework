#ifndef VSTREAMTHREAD_H
#define VSTREAMTHREAD_H

#include <thread>
#include <cx_net_sockets/socket_base_stream.h>

/**
 * Class for managing the client on his thread.
 */
class StreamThread
{
public:
    /**
     * constructor
     */
    StreamThread();
    /**
     * destructor
     */
    ~StreamThread();
    /**
     * Start the thread of the client.
     */
    void start();
    /**
     * Kill the client socket
     */
    void stopSocket();
    /**
     * Set parent (stream acceptor object)
     * @param parent parent
     */
    void setParent(void * parent);
    /**
     * Set callback when connection is fully established
     */
    void setCallbackOnConnect(bool (*_callbackOnConnect)(void *, Socket_Base_Stream *, const char *), void *objOnConnected);
    /**
     * Set callback when protocol initialization failed (like bad X.509 on TLS)
     */
    void setCallbackOnInitFail(bool (*_callbackOnInitFailed)(void *, Socket_Base_Stream *, const char *), void *objOnConnected);
    /**
     * Call callback
     * to be used from the client thread.
     */
    void postInitConnection();
    /**
     * Set socket
     */
    void setSocket(Socket_Base_Stream * _clientSocket);

    const char * getRemotePair();

private:
    Socket_Base_Stream * clientSocket;
    bool (*callbackOnConnect)(void *,Socket_Base_Stream *, const char *);
    bool (*callbackOnInitFail)(void *,Socket_Base_Stream *, const char *);

    char remotePair[INET6_ADDRSTRLEN+2];

    void *objOnConnect, *objOnInitFail;
    void * parent;
};

#endif // VSTREAMTHREAD_H
