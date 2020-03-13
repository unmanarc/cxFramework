#ifndef SOCKET_H
#define SOCKET_H

#include <sys/socket.h>
#include <netinet/in.h>

#include <stdint.h>
#include <unistd.h>

#include <memory>
#include <atomic>

enum SocketMode
{
    DATAGRAM_SOCKET,
    STREAM_SOCKET,
    UNINITIALIZED_SOCKET
};

struct NanoSocket
{
    NanoSocket()
    {
        sockfd=-1;
    }
    ~NanoSocket()
    {
        // close the socket when .
        if (sockfd!=-1)
        {
            close(sockfd);
            sockfd = -1;
        }
    }
    bool isActive()
    {
        return sockfd!=-1;
    }
    int sockfd;
};

/**
 * Socket base class
 * Manipulates all kind of sockets (udp,tcp,unix, etc)
 */
class Socket {
public:
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors/Destructors/Copy:
    /**
     * Socket Class Constructor
     * note: does not initialize the socket
     */
    Socket();
    Socket(int _sockfd);
    virtual ~Socket();

#ifdef _WIN32
    static bool win32Init();
#endif
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Socket Options:
    /**
     * Set to use write
     * use write instead send.
     */
    void setUseWrite();
    /**
     * Set Read timeout.
     * @param _timeout timeout in seconds
     */
    bool setReadTimeout(unsigned int _timeout);
    /**
     * Set Write timeout.
     * @param _timeout timeout in seconds
     */
    bool setWriteTimeout(unsigned int _timeout);
    /**
     * Set system buffer size.
     * Use to increase the current reception buffer
     * @param buffsize buffer size in bytes.
     */
    void setRecvBuffer(int buffsize);
    /**
     * @brief setBlockingMode Set Socket Blocking Mode
     * @param blocking mode (false: non-blocking)
     * @return true if succeeed
     */
    bool setBlockingMode(bool blocking = true);
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Socket Manipulation:
    /**
     * Set socket file descriptor value.
     * This is useful to manipulate already created sockets.
     * @param _sockfd socket file descriptor
     */
    void setSocket(int _sockfd);
    /**
     * Get Current Socket file descriptor
     * @return socket file descriptor
     */
    int getSocket() const;

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Socket Status:
    /**
     * Check if we have an initialized socket.
     * @return true if the socket is a valid file descriptor
     */
    bool isActive() const;
    /**
     * Check if the remote pair is connected or not.
     * @param true if is it connected
     */
    virtual bool isConnected();
    /**
     * Get current used port.
     * Useful for TCP/UDP connections, especially on received connections.
     * @param 16-bit unsigned integer with the port (0-65535)
     */
    uint16_t getPort();
    /**
     * Get last error message
     * @param last error message pointer. (static mem)
     */
    const char * getLastError() const;
    /**
     * Get remote pair address
     * @param address pair address char * (should contain at least 64 bytes)
     */
    void getRemotePair(char * address) const;
    /**
     * Set remote pair address
     * Used by internal functions...
     * @param address pair address char * (should contain at least 64 bytes)
     */
    void setRemotePair(const char * address);
    /**
     * @brief getRemotePort Get Remote Port for listenning connections
     * @return remote port 0-65535
     */
    unsigned short getRemotePort() const;
    /**
     * Set remote port
     * Used by internal functions...
     * @param value port
     */
    void setRemotePort(unsigned short value);

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Socket Options...
    /**
     * @brief getSockOpt Get Socket Option
     * @param level IP / TCP / ...
     * @param optname SO Option Name
     * @param optval Option Value Pointer
     * @param optlen IN/OUT option Value Memory Length
     * @return zero if succeed. error -1
     */
    int getSockOpt(int level, int optname, void *optval, socklen_t *optlen);
    /**
     * @brief setSockOpt Set Socket Option
     * @param level IP / TCP / ...
     * @param optname SO Option Name
     * @param optval Option Value Pointer
     * @param optlen Option Value Memory Length
     * @return zero if succeed. error -1
     */
    int setSockOpt(int level, int optname, const void *optval, socklen_t optlen);

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Socket Connection elements...
    /**
     * Connect to remote host
     * This is a virtual function
     * @param hostname remote hostname to connect to
     * @param port 16-bit unsigned integer with the remote port
     * @param timeout timeout in seconds to desist the connection.
     * @return true if successfully connected
     */
    virtual bool connectTo(const char * hostname, const  uint16_t & port, const uint32_t & timeout);
    /**
     * Try connect until it succeeds.
     */
    void tryConnect(const char * hostname, const uint16_t & port, const uint32_t & timeout);
    /**
     * Listen on an specific port and address
     * @param listenOnAddress address to listen on. (use :: for ipv6 or 0.0.0.0 if ipv4)
     * @param port 16-bit unsigned integer with the listening port (0-65535)
     * @return true if we can bind the port.
     */
    virtual bool listenOn(const uint16_t & port, const char * listenOnAddr);

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Connection Finalization...
    /**
     * Will loose the control of the socket descriptor (be careful)
     */
    void detachSocket();
    /**
     * Close the socket itself (will close the connection)
     */
    int closeSocket();
    /**
     * Shutdown the connection
     * Use for terminate the connection. both sides will start to fail to receive/send
     * You should also use Close too. (or reuse the socket?)
     */
    virtual int shutdownSocket(int mode = SHUT_RDWR);
    /**
     * Read a data block from the socket
     * Receive the data block in only one command (without chunks).
     * note that this haves some limitations. some systems can only receive 4k at time.
     * You may want to manage the chunks by yourself.
     * @param data data block.
     * @param datalen data length in bytes
     * @return return the number of bytes read by the socket, zero for end of file and -1 for error.
     */
    virtual int partialRead(void * data, const uint32_t & datalen);
    /**
     * Write a data block to the socket
     * note that this haves some limitations. some systems can only send 4k at time.
     * You may want to manage the chunks by yourself.
     * @param data data block.
     * @param datalen data length in bytes
     * @return return the number of bytes read by the socket, zero for end of file and -1 for error.
     */
    virtual int partialWrite(const void *data, const uint32_t & datalen);

private:
    static void socketSystemInitialization();
    void initVars();

protected:
    /**
     * if true, Use write instead send and read instead recv.
     */
    bool useWrite;
    /**
     * pointer to the last error message.
     */
    const char * lastError;
    /**
     * buffer with the remote pair address.
     */
    char remotePair[INET6_ADDRSTRLEN+1];
    /**
     * @brief remotePort remote port when accepting connections.
     */
    unsigned short remotePort;
    /**
     * Shared pointer to socket file descriptor.
     */
    std::shared_ptr<NanoSocket> sharedSocket;

    static bool socketInitialized,badSocket;

    std::atomic<unsigned int> readTimeout;
    std::atomic<unsigned int> writeTimeout;
    std::atomic<unsigned int> recvBuffer;

    bool listenMode;

#ifdef _WIN32
    static bool winSockInitialized;
#endif
};

#endif // SOCKET_H
