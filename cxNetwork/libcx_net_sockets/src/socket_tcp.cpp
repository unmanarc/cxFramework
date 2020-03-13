#include "socket_tcp.h"

#ifdef _WIN32
#include "win32compat/win32netcompat.h"
#include <winsock2.h>
#include <Ws2tcpip.h>
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <netdb.h>
#endif

#include <sys/types.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>

Socket_TCP::Socket_TCP()
{
    ovrReadTimeout = -1;
    ovrWriteTimeout = -1;
}

Socket_TCP::~Socket_TCP()
{
//    printf("deleting socket %p\n", this); fflush(stdout);
}

bool Socket_TCP::connectTo(const char * hostname, const uint16_t &port, const uint32_t &timeout)
{
    char serverPort[32];
    int rc;
    struct in6_addr serveraddr;
    struct addrinfo hints, *res=nullptr;

    memset(&hints, 0x00, sizeof(hints));

#ifdef _WIN32
    hints.ai_flags    = 0;
#else
    hints.ai_flags    = AI_NUMERICSERV;
#endif
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family   = AF_UNSPEC;

    rc = inet_pton(AF_INET, hostname, &serveraddr);
    if (rc == 1)
    {
        hints.ai_family = AF_INET;
        hints.ai_flags |= AI_NUMERICHOST;
    }
    else
    {
        rc = inet_pton(AF_INET6, hostname, &serveraddr);
        if (rc == 1)
        {
            hints.ai_family = AF_INET6;
            hints.ai_flags |= AI_NUMERICHOST;
        }
    }

    snprintf(serverPort,32,"%u",port);

    rc = getaddrinfo(hostname, serverPort, &hints, &res);
    if (rc != 0)
    {
        // Host not found.
        lastError = "Error resolving hostname";
        return false;
    }

    bool connected = false;

    for (struct addrinfo *resiter=res; resiter && !connected; resiter = resiter->ai_next)
    {
        if (getSocket() >=0 ) closeSocket();
        setSocket(socket(res->ai_family, res->ai_socktype, res->ai_protocol));
        if (!isActive())
        {
            lastError = "socket() failed";
            break;
        }

        // Set the read timeout here. (to zero)
        setReadTimeout(0);

        sockaddr * curAddr = resiter->ai_addr;
        struct sockaddr_in * curAddrIn = ((sockaddr_in *)curAddr);

        if (tcpConnect(curAddr, resiter->ai_addrlen,timeout))
        {

            if (ovrReadTimeout!=-1) setReadTimeout(ovrReadTimeout);
            if (ovrWriteTimeout!=-1) setWriteTimeout(ovrWriteTimeout);

            // Set remote pairs...
            switch (curAddr->sa_family)
            {
            case AF_INET:
            {
                char ipAddr4[INET_ADDRSTRLEN+1]="";
                inet_ntop(AF_INET, &(curAddrIn->sin_addr), ipAddr4, INET_ADDRSTRLEN);
                setRemotePair(ipAddr4);
            } break;
            case AF_INET6:
            {
                char ipAddr6[INET6_ADDRSTRLEN+1]="";
                inet_ntop(AF_INET6, &(curAddrIn->sin_addr), ipAddr6, INET6_ADDRSTRLEN);
                setRemotePair(ipAddr6);
            } break;
            default:
                break;
            }

            setRemotePort(port);

            // now it's connected...
            if (postConnectSubInitialization())
            {
                connected = true;
            }
            else
            {
            	// should disconnect here.
            	shutdownSocket();
                // drop the socket descriptor. we don't need it anymore.
                closeSocket();
            }
            break;
        }
        else
        {
            // drop the current socket... (and free the resource :))
            shutdownSocket();
            closeSocket();
        }
    }

    freeaddrinfo(res);

    if (!connected)
    {
        lastError = "connect() failed";
        return false;
    }

    return true;
}

Socket_Base_Stream * Socket_TCP::acceptConnection()
{
    int sdconn;

    if (!isActive()) return nullptr;

    Socket_Base_Stream * cursocket;

    int32_t clilen;
    struct sockaddr_in cli_addr;
    clilen = sizeof(cli_addr);

    if ((sdconn = accept(getSocket(), (struct sockaddr *) &cli_addr, (socklen_t *)&clilen)) >= 0)
    {
        cursocket = new Socket_TCP;
        // Set the proper socket-
        cursocket->setSocket(sdconn);
        char ipAddr[INET6_ADDRSTRLEN+1];
        inet_ntop(AF_INET, &cli_addr.sin_addr, ipAddr, sizeof(ipAddr)-1);
        cursocket->setRemotePort(ntohs(cli_addr.sin_port));
        cursocket->setRemotePair(ipAddr);

        if (readTimeout) cursocket->setReadTimeout(readTimeout);
        if (writeTimeout) cursocket->setWriteTimeout(writeTimeout);
        if (recvBuffer) cursocket->setRecvBuffer(recvBuffer);
    }
    // Establish the error.
    else
    {
        lastError = "accept() failed";
        return nullptr;
    }

    // return the socket class.
    return cursocket;
}

bool Socket_TCP::tcpConnect(const sockaddr *addr, socklen_t addrlen, const uint32_t &timeout)
{
    int res2,valopt;

    // Non-blocking connect with timeout...
    if (!setBlockingMode(false)) return false;

#ifdef _WIN32
    if (timeout == 0)
    {
        // in windows, if the timeval is 0,0, then it will return immediately.
        // however, our lib state that 0 represent that we sleep for ever.
        timeout = 365*24*3600; // how about 1 year.
    }
#endif

    // Trying to connect with timeout.
    res2 = connect(getSocket(), addr, addrlen);
    if (res2 < 0)
    {
        if (errno == EINPROGRESS || !errno)
        {
            fd_set myset;

            struct timeval tv;
            tv.tv_sec = timeout;
            tv.tv_usec = 0;
            FD_ZERO(&myset);
            FD_SET(getSocket(), &myset);

            res2 = select(getSocket()+1, nullptr, &myset, nullptr, timeout?&tv:nullptr);

            if (res2 < 0 && errno != EINTR)
            {
                lastError = "Error selecting...";
                return false;
            }
            else if (res2 > 0)
            {
                // Socket selected for write
                socklen_t lon;
                lon = sizeof(int);
#ifdef _WIN32
                if (getsockopt(getSocket(), SOL_SOCKET, SO_ERROR, (char*)(&valopt), &lon) < 0)
#else
                if (getsockopt(getSocket(), SOL_SOCKET, SO_ERROR, (void*)(&valopt), &lon) < 0)
#endif
                {
                    lastError = "Error in getsockopt(SOL_SOCKET)";
                    return false;
                }
                // Check the value returned...
                if (valopt)
                {
                    lastError = "Error in delayed connection()";
                    return false;
                }

                // Even if we are connected, if we can't go back to blocking, disconnect.
                if (!setBlockingMode(true)) return false;

                // Connected!!!
                // Pass to blocking mode socket instead select it.
                return true;
            }
            else
            {
                lastError = "Timeout in select() - Cancelling!";
                return false;
            }
        }
        else
        {
            lastError = "Error connecting - (2)";
            return false;
        }
    }
    // What we are doing here?
    setBlockingMode(true);
    return false;
}

bool Socket_TCP::listenOn(const uint16_t & port, const char * listenOnAddr, bool useIPv4, const int32_t & recvbuffer,const int32_t & backlog)
{
    int on=1;

    if (useIPv4)
    {
        setSocket(socket(AF_INET, SOCK_STREAM, 0));
        if (!isActive())
        {
            lastError = "socket() failed";
            return false;
        }
    }
    else
    {
        setSocket(socket(AF_INET6, SOCK_STREAM, 0));
        if (!isActive())
        {
            lastError = "socket() failed";
            return false;
        }
    }

    if (recvbuffer) setRecvBuffer(recvbuffer);

    if (setsockopt(getSocket(), SOL_SOCKET, SO_REUSEADDR, static_cast<void *>(&on),sizeof(on)) < 0)
    {
        lastError = "setsockopt(SO_REUSEADDR) failed";
        closeSocket();
        return false;
    }

    if (useIPv4)
    {
        struct sockaddr_in serveraddr;
        memset(&serveraddr, 0, sizeof(serveraddr));

        serveraddr.sin_family = AF_INET;
        serveraddr.sin_port   = htons(port);

        inet_pton(AF_INET, listenOnAddr, &serveraddr.sin_addr);

        if (bind(getSocket(),(struct sockaddr *)&serveraddr,sizeof(serveraddr)) < 0)
        {
            lastError = "bind() failed";
            closeSocket();
            return false;
        }
    }
    else
    {
        struct sockaddr_in6 serveraddr;
        memset(&serveraddr, 0, sizeof(serveraddr));

        serveraddr.sin6_family = AF_INET6;
        serveraddr.sin6_port   = htons(port);

        inet_pton(AF_INET6, listenOnAddr, &serveraddr.sin6_addr);

        if (bind(getSocket(),(struct sockaddr *)&serveraddr,sizeof(serveraddr)) < 0)
        {
            lastError = "bind() failed";
            closeSocket();
            return false;
        }
    }

    if (listen(getSocket(), backlog) < 0)
    {
        lastError = "listen() failed";
        closeSocket();
        return false;
    }


    listenMode = true;

    return true;
}

bool Socket_TCP::postAcceptSubInitialization()
{
    return true;
}

int Socket_TCP::setTCPOptionBool(const int32_t &optname, bool value)
{
    int flag = value?1:0;
    return setTCPOption(optname, (char *) &flag, sizeof(int));
}

int Socket_TCP::setTCPOption(const int32_t &optname, const void *optval, socklen_t optlen)
{
    return setSockOpt(IPPROTO_TCP, optname, optval, optlen);
}

int Socket_TCP::getTCPOption(const int32_t & optname, void *optval, socklen_t *optlen)
{
    return getSockOpt(IPPROTO_TCP, optname, optval, optlen);
}

void Socket_TCP::overrideReadTimeout(int32_t tout)
{
    ovrReadTimeout = tout;
}

void Socket_TCP::overrideWriteTimeout(int32_t tout)
{
    ovrWriteTimeout = tout;
}
/*
bool Socket_TCP::postConnectSubInitialization()
{
	return true;
}
*/
