#include "socket.h"

#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>

#ifdef _WIN32
#include <winsock2.h>
#include <Ws2tcpip.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#endif

#ifdef _WIN32
// Init winsock when the program begins...
bool Socket::winSockInitialized = Socket::win32Init();
#endif
bool Socket::socketInitialized = false;
bool Socket::badSocket = false;


void Socket::initVars()
{
    listenMode = false;
    readTimeout = 0;
    writeTimeout = 0;
    recvBuffer = 0;
    useWrite = false;
    lastError = "";
    // Create an invalid socket...
    sharedSocket.reset(new NanoSocket);
    memset(remotePair, 0, sizeof(remotePair));
}

Socket::Socket()
{
    initVars();
}

Socket::Socket(int _sockfd)
{
    initVars();
    setSocket(_sockfd);
}

Socket::~Socket()
{
    //closeSocket();
}

#ifdef _WIN32
bool Socket::win32Init()
{
    socketSystemInitialization();
}
#endif

void Socket::setUseWrite()
{
	// prevent the application from crash, ignore the sigpipes:
#ifndef _WIN32
	signal(SIGPIPE, SIG_IGN);
#endif
	// use write/read functions instead send/recv
	useWrite = true;
}

void Socket::setRecvBuffer(int buffsize)
{
    recvBuffer = buffsize;

    if (!sharedSocket->isActive()) return;
#ifdef _WIN32
    setsockopt(nanoSocket->sockfd, SOL_SOCKET, SO_RCVBUF, (char *) &buffsize, sizeof(buffsize));
#else
    setsockopt(sharedSocket->sockfd, SOL_SOCKET, SO_RCVBUF, &buffsize, sizeof(buffsize));
#endif
}

bool Socket::isConnected()
{
	return false;
}

bool Socket::connectTo(const char *, const uint16_t &, const uint32_t &)
{
	return false;
}

void Socket::tryConnect(const char *hostname, const uint16_t &port,
        const uint32_t &timeout)
{
	while (!connectTo(hostname, port, timeout))
	{
		// Try to reconnect if fail...
	}
}

bool Socket::listenOn(const uint16_t &, const char *)
{
    return false;
}

void Socket::detachSocket()
{
    sharedSocket->sockfd = -1;
}

int Socket::closeSocket()
{
    if (!sharedSocket->isActive()) return 0;
#ifdef _WIN32
    int i = closesocket(nanoSocket->sockfd);
#else
    int i = close(sharedSocket->sockfd);
#endif
    sharedSocket->sockfd = -1;
    return i;
}

const char * Socket::getLastError() const
{
	return lastError;
}

void Socket::setSocket(int _sockfd)
{
    if (getSocket() != -1)
	{
		// PUT YOUR WARN ABOUT REWRITE:
	}
    sharedSocket->sockfd = _sockfd;
}

uint16_t Socket::getPort()
{
    if (!sharedSocket->isActive()) return 0;

	struct sockaddr_in sin;
	socklen_t len = sizeof(sin);
    if (getsockname(sharedSocket->sockfd, (struct sockaddr *) &sin, &len) == -1)
	{
		lastError = "Error resolving port";
		return 0;
	}
	return ntohs(sin.sin_port);
}

int Socket::partialRead(void *data, const uint32_t &datalen)
{
    if (!sharedSocket->isActive()) return -1;
    if (!datalen) return 0;
    if (!useWrite)
	{
        ssize_t recvLen = recv(sharedSocket->sockfd, (char *) data, datalen, 0);
		return recvLen;
	}
	else
	{
        ssize_t recvLen = read(sharedSocket->sockfd, (char *) data, datalen);
		return recvLen;
    }
}

int Socket::partialWrite(const void *data, const uint32_t &datalen)
{
    if (!sharedSocket->isActive()) return -1;
    if (!datalen) return 0;
    if (!useWrite)
    {
#ifdef _WIN32
        ssize_t sendLen = send(nanoSocket->sockfd, (char *) data, datalen, 0);
#else
        ssize_t sendLen = send(sharedSocket->sockfd, (char *) data, datalen, MSG_NOSIGNAL);
#endif
        return sendLen;
    }
    else
    {
        ssize_t sendLen = write(sharedSocket->sockfd, (char *) data, datalen);
        return sendLen;
    }
}

void Socket::socketSystemInitialization()
{
    if (!socketInitialized)
    {
#ifdef _WIN32
        int wsaerr;

        WORD wVersionRequested;
        WSADATA wsaData;

        wVersionRequested = MAKEWORD(2, 2);
        wsaerr = WSAStartup(wVersionRequested, &wsaData);
        if (wsaerr != 0)
        {
            // dll not found.
            badSocket = true;
            return;
        }

        if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2 )
        {
            // not supported.
            WSACleanup();
            badSocket = true;
            return;
        }
#endif
        socketInitialized = true;
    }
}


unsigned short Socket::getRemotePort() const
{
    return remotePort;
}

void Socket::setRemotePort(unsigned short value)
{
    remotePort = value;
}

int Socket::getSockOpt(int level, int optname, void *optval, socklen_t *optlen)
{
    return getsockopt(sharedSocket->sockfd, level, optname, optval, optlen);
}

int Socket::setSockOpt(int level, int optname, const void *optval, socklen_t optlen)
{
    return setsockopt(sharedSocket->sockfd,  level, optname, optval, optlen);
}

bool Socket::setReadTimeout(unsigned int _timeout)
{
    if (!sharedSocket->isActive()) return false;

    readTimeout = _timeout;

    if (listenMode) return true;

#ifdef _WIN32
    DWORD tout = _timeout*1000;
    if ((setsockopt(nanoSocket->sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&tout, sizeof(DWORD))) == -1)
#else
    struct timeval timeout;
    timeout.tv_sec = _timeout;
    timeout.tv_usec = 0;
    if ((setsockopt(sharedSocket->sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout))) == -1)
#endif
	{
		return false;
	}
	return true;
}

bool Socket::setWriteTimeout(unsigned int _timeout)
{
    if (!sharedSocket->isActive()) return false;
    writeTimeout = _timeout;
    if (listenMode) return true;
#ifdef _WIN32
    int tout = _timeout*1000;
    if ((setsockopt(nanoSocket->sockfd, SOL_SOCKET, SO_SNDTIMEO, (char *)&tout, sizeof(int))) == -1)
#else
    struct timeval timeout;
    timeout.tv_sec = _timeout;
    timeout.tv_usec = 0;
    if ((setsockopt(sharedSocket->sockfd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout))) == -1)
#endif
    {
        return false;
    }
    return true;
}

bool Socket::isActive() const
{
    return sharedSocket->isActive();
}

int Socket::getSocket() const
{
    return sharedSocket->sockfd;
}

void Socket::getRemotePair(char * address) const
{
    strncpy(address, remotePair, INET6_ADDRSTRLEN);
}

void Socket::setRemotePair(const char * address)
{
    remotePair[INET6_ADDRSTRLEN] = 0;
    strncpy(remotePair, address, INET6_ADDRSTRLEN);
}

int Socket::shutdownSocket(int mode)
{
    if (!sharedSocket->isActive()) return -1;
    //printf("Shutting down %d\n", sharedSocket->sockfd); fflush(stdout);
    return shutdown(sharedSocket->sockfd, mode);
}

bool Socket::setBlockingMode(bool blocking)
{
#ifdef _WIN32
    int iResult;
    unsigned long int iMode = (!blocking)?1:0;
    iResult = ioctlsocket(nanoSocket->sockfd, FIONBIO, &iMode);
    return (iResult == NO_ERROR);
#else
    long arg;
    // Set to blocking mode again...
    if( (arg = fcntl(sharedSocket->sockfd, F_GETFL, nullptr)) < 0)
    {
        lastError = "Error getting blocking mode... ";
        return false;
    }
    if (blocking)
        arg &= (~O_NONBLOCK);
    else
        arg |= (O_NONBLOCK);

    if( fcntl(sharedSocket->sockfd, F_SETFL, arg) < 0)
    {
        lastError = "Error setting blocking...";
        return false;
    }
    return true;
#endif
}
