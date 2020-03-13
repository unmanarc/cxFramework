#include "w32compat.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <winsock2.h>
#include <ws2tcpip.h>


const char *inet_ntop(int af, const void *src, char *dst, socklen_t size)
{
    struct sockaddr_storage addrStorage;
    unsigned long s = size;

    ZeroMemory(&addrStorage, sizeof(addrStorage));

    addrStorage.ss_family = af;

    switch(af)
    {
    case AF_INET6:
        ((struct sockaddr_in6 *)&addrStorage)->sin6_addr = *(struct in6_addr *)src;
        break;
    case AF_INET:
        ((struct sockaddr_in *)&addrStorage)->sin_addr = *(struct in_addr *)src;
        break;
    default:
        return nullptr;
    }

    if (!WSAAddressToStringA((struct sockaddr *)&addrStorage, sizeof(addrStorage), nullptr, dst, &s))
        return dst;
    return nullptr;
}

int inet_pton(int af, const char *src, void *dst)
{
    struct sockaddr_storage addrStorage;
    int size = sizeof(addrStorage);
    char srcv2[INET6_ADDRSTRLEN+1];

    ZeroMemory(&addrStorage, sizeof(addrStorage));

    srcv2[INET6_ADDRSTRLEN]=0;
    strncpy(srcv2, src, INET6_ADDRSTRLEN);

    if (WSAStringToAddressA(srcv2, af, nullptr, (struct sockaddr *)&addrStorage, &size) == 0)
    {
        switch(af)
        {
        case AF_INET6:
            *(struct in6_addr *)dst = ((struct sockaddr_in6 *)&addrStorage)->sin6_addr;
            return 1;
        case AF_INET:
            *(struct in_addr *)dst = ((struct sockaddr_in *)&addrStorage)->sin_addr;
            return 1;
        }
    }
    return 0;
}
