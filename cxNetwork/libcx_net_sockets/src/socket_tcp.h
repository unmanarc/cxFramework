#ifndef SOCKET_TCP_H
#define SOCKET_TCP_H

#include "socket_base_stream.h"
#include <unistd.h>

#ifdef _WIN32
#include <ws2tcpip.h>
#endif

/**
 * TCP Socket Class
 */
class Socket_TCP : public Socket_Base_Stream {
public:
    /**
     * Class constructor.
     */
    Socket_TCP();
    virtual ~Socket_TCP();
    /**
     * Listen on an specific TCP port and address
     * @param listenOnAddress address to listen on. (use :: for ipv6 or 0.0.0.0 if ipv4)
     * @param port 16-bit unsigned integer with the listening TCP port (1-65535), 0 means random available port.
     * @return true if the operation succeeded.
     */
    bool listenOn(const uint16_t & port, const char * listenOnAddr = "::", bool useIPv4 =false, const int32_t &recvbuffer = 0, const int32_t &backlog = 10);
    /**
     * Connect to remote host using a TCP socket.
     * @param hostname remote hostname to connect to, can be the hostname or the ip address
     * @param port 16-bit unsigned integer with the remote port
     * @param timeout timeout in seconds to desist the connection. (default 30)
     * @return true if successfully connected
     */
    bool connectTo(const char * hostname, const uint16_t & port, const uint32_t & timeout = 30);
    /**
     * Accept a new TCP connection on a listening socket.
     * @return returns a socket with the new established tcp connection.
     */
    virtual Socket_Base_Stream *acceptConnection();

    /**
     * Virtual function for protocol initialization after the connection starts...
     * useful for SSL server, it runs in blocking mode and should be called apart to avoid tcp accept while block
     * @return returns true if was properly initialized.
     */
    virtual bool postAcceptSubInitialization();

    int setTCPOptionBool(const int32_t & optname, bool value = true);
    int setTCPOption(const int32_t & optname,const void *optval, socklen_t optlen);
    int getTCPOption(const int32_t &optname, void *optval, socklen_t * optlen);


    void overrideReadTimeout(int32_t tout = -1);
    void overrideWriteTimeout(int32_t tout = -1);

protected:


private:
    bool tcpConnect(const struct sockaddr *addr, socklen_t addrlen, const uint32_t & timeout);

    int32_t ovrReadTimeout,ovrWriteTimeout;

};

#endif // SOCKET_TCP_H
