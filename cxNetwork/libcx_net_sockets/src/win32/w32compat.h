#ifndef WIN32NETCOMPAT_H
#define WIN32NETCOMPAT_H

#include <Ws2tcpip.h>

int inet_pton(int af, const char *src, void *dst);
const char *inet_ntop(int af, const void *src, char *dst, socklen_t size);

#endif // WIN32NETCOMPAT_H
