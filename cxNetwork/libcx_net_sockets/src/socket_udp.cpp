#include "socket_udp.h"
#include <sys/types.h>
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include "win32compat/win32netcompat.h"
#include <winsock2.h>
#include <Ws2tcpip.h>
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif

Socket_UDP::Socket_UDP()
{
	res = nullptr;
}

Socket_UDP::~Socket_UDP()
{
	if (res)
		freeaddrinfo(res);
	res = nullptr;
}

bool Socket_UDP::isConnected()
{
	return true;
}

bool Socket_UDP::listenOn(const uint16_t & port, const char *listenOnAddr)
{
	int on = 1;
	struct sockaddr_in6 serveraddr;

    if (isActive()) closeSocket(); // close and release first
	// Socket initialization.
    setSocket(socket(AF_INET6, SOCK_DGRAM, 0));
    if (!isActive())
	{
		lastError = "socket() failed";
		return false;
	}

	// Set to reuse address (if released and wait)..
    if (setsockopt(getSocket(), SOL_SOCKET, SO_REUSEADDR, (char *) &on, sizeof(on)) < 0)
	{
		lastError = "setsockopt(SO_REUSEADDR) failed";
		closeSocket();
		return false;
	}

	// Initialize memory structures
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin6_family = AF_INET6;
	serveraddr.sin6_port = htons(port);

	// Server hostname to network address
	inet_pton(AF_INET6, listenOnAddr, &serveraddr.sin6_addr);

	// Bind on socket.
    if (bind(getSocket(), (struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0)
	{
		lastError = "bind() failed";
		closeSocket();
		return false;
	}

    listenMode = true;

	// Done!
	return true;
}

bool Socket_UDP::connectTo(const char * hostname, const uint16_t & port, const uint32_t & timeout)
{
    if (isActive()) closeSocket(); // close and release first

	char servport[32];
	snprintf(servport, 32, "%u", port);

	int rc;
	struct in6_addr serveraddr;
	struct addrinfo hints;

	// Initialize the memory.
	memset(&hints, 0x00, sizeof(hints));
#ifdef _WIN32
    hints.ai_flags = 0;
#else
    hints.ai_flags = AI_NUMERICSERV;
#endif
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;

	// Translate the hostname to network address (as ipv4)
	rc = inet_pton(AF_INET, hostname, &serveraddr);
	if (rc == 1)
	{
		// Use IPv4.
		hints.ai_family = AF_INET;
		hints.ai_flags |= AI_NUMERICHOST;
	}
	else
	{
		// Try to translate on IPv6.
		rc = inet_pton(AF_INET6, hostname, &serveraddr);
		if (rc == 1)
		{
			// Use IPv6.
			hints.ai_family = AF_INET6;
			hints.ai_flags |= AI_NUMERICHOST;
		}
	}

	// Check the host existance.
	rc = getaddrinfo(hostname, servport, &hints, &res);
	if (rc != 0)
	{
		// Host not found.
		lastError = "Error resolving hostname";
		return false;
	}

	// Initialize the socket...
    setSocket(socket(res->ai_family, res->ai_socktype, res->ai_protocol));
    if (!isActive())
	{
		lastError = "socket() failed";
		return false;
	}

	// Set the timeout here.
	setReadTimeout(timeout);

	// UDP connection does not establish the connection. is enough to have the remote address resolved and the socket file descriptor...
	bool connected = (res ? true : false);
	if (!connected)
	{
		lastError = "connect() failed on addr resolution";
		return false;
	}

	return true;
}

bool Socket_UDP::writeBlock(const void *data, const uint32_t & datalen)
{
    if (!isActive()) return false;
	if (!res) return false;
#ifdef _WIN32
    if (sendto(getSocket(), (char *)data, datalen, 0, res->ai_addr, res->ai_addrlen) == -1)
#else
    if (sendto(getSocket(), data, datalen, 0, res->ai_addr, res->ai_addrlen) == -1)
#endif
	{
		return false;
	}
	return true;
}

#define SOCKADDR_IN_SIZE sizeof(struct sockaddr)

uint32_t Socket_UDP::getMinReadSize()
{
	return (SOCKADDR_IN_SIZE + sizeof(int));
}

bool Socket_UDP::readBlock(void *data, const uint32_t &datalen)
{
    return false; // USE: readDatagramBlock
}

std::shared_ptr<DatagramBlock> Socket_UDP::readDatagramBlock()
{
    std::shared_ptr<DatagramBlock> datagramBlock;
    datagramBlock.reset(new DatagramBlock);
    if (!isActive()) return datagramBlock;

    socklen_t fromlen = SOCKADDR_IN_SIZE;

    char bigBlock[65536];
#ifdef _WIN32
    (*datagramBlock).datalen = recvfrom(getSocket(), bigBlock, 65536, 0, &((*datagramBlock).addr) , &fromlen);
#else
    (*datagramBlock).datalen = recvfrom(getSocket(), (void *) bigBlock, 65536, 0, &((*datagramBlock).addr) , &fromlen);
#endif

    (*datagramBlock).copy(bigBlock, (*datagramBlock).datalen);
    return datagramBlock;
}

