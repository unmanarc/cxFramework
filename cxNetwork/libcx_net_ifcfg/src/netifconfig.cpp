#include "netifconfig.h"
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>

#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/if_ether.h> /* includes net/ethernet.h */


NetIfConfig::NetIfConfig()
{
    address_setted = true;
    MTU = 0;
    fd = -1;
}

NetIfConfig::~NetIfConfig()
{
    if (fd>=0)
        close(fd);
}

bool NetIfConfig::openInterface(const std::string &_ifaceName)
{
    interfaceName = _ifaceName;
    if((fd = socket(AF_INET, SOCK_RAW, IPPROTO_TCP)) < 0)
    {
        lastError = "socket error @" + _ifaceName;
        return false;
    }
    strncpy(ifr.ifr_name, _ifaceName.c_str(),IFNAMSIZ);
    if((ioctl(fd, SIOCGIFFLAGS, &ifr) == -1))
    {
        lastError = "SIOCGIFFLAGS error @" + _ifaceName;
        return false;
    }

    return true;
}

int NetIfConfig::getMTU()
{
    struct ifreq ifr2;
    int sock2;
    if((sock2 = socket(AF_INET, SOCK_RAW, IPPROTO_TCP)) < 0)
    {
        return 0;
    }
    strncpy(ifr2.ifr_name, interfaceName.c_str(),IFNAMSIZ);
    if((ioctl(sock2, SIOCGIFMTU, &ifr2) == -1))
    {
        lastError = "SIOCGIFMTU error @" + interfaceName;
        return false;
    }
    close(sock2);
    return ifr2.ifr_mtu;

}

ethhdr NetIfConfig::getEthernetAddress()
{
    ethhdr x;
    x.h_proto = htons(ETH_P_IP);
    memset(x.h_dest,0,6);
    memset(x.h_source,0,6);
    struct ifreq ifr1x = ifr;
    if (ioctl(fd, SIOCGIFHWADDR, &ifr1x) < 0)
    {
        lastError = "SIOCGIFHWADDR error @" + interfaceName;
        return x;
    }
    memcpy(x.h_dest,ifr1x.ifr_hwaddr.sa_data,6);
    return x;
}

std::string NetIfConfig::getLastError() const
{
    return lastError;
}

void NetIfConfig::setIPv4Address(const in_addr &_address, const in_addr &_netmask)
{
    address_setted = false;

    sockaddr_in addr;
    memset(&addr,0,sizeof(sockaddr_in));

    ifr1_host = ifr;
    ifr1_netmask = ifr;

    addr.sin_addr.s_addr = _address.s_addr;
    addr.sin_family = AF_INET;
    memcpy(&ifr1_host.ifr_addr, &addr, sizeof(addr));

    addr.sin_addr.s_addr = _netmask.s_addr;
    memcpy(&ifr1_netmask.ifr_netmask, &addr, sizeof(addr));
}

void NetIfConfig::setMTU(int _mtu)
{
    MTU = _mtu;
}

void NetIfConfig::setPromiscMode(bool state)
{
    // set the flags to PROMISC
    if (state)
        ifr.ifr_flags |= (IFF_PROMISC);
    else
        ifr.ifr_flags &= ~(IFF_PROMISC);
}

void NetIfConfig::setUP(bool state)
{
    // set the flags to UP...
    if (state)
        ifr.ifr_flags |= (IFF_UP | IFF_RUNNING);
    else
        ifr.ifr_flags &= ~(IFF_UP | IFF_RUNNING);
}

bool NetIfConfig::apply()
{
    if (!address_setted)
    {
        if (ioctl(fd,SIOCSIFADDR,&ifr1_host) == -1)
        {
            lastError = "SIOCSIFADDR error @" + interfaceName;
            return false;
        }
        if (ioctl(fd,SIOCSIFNETMASK,&ifr1_netmask) == -1)
        {
            lastError = "SIOCSIFNETMASK error @" + interfaceName;
            return false;
        }
        address_setted = true;
    }

    // promisc and up:
    if (ioctl (fd, SIOCSIFFLAGS, &ifr) == -1 )
    {
        lastError = "SIOCSIFFLAGS error @" + interfaceName;
        return false;
    }

    if (MTU!=0)
    {
        ifr.ifr_mtu = MTU;
        if (ioctl(fd, SIOCSIFMTU, &ifr) < 0)
        {
            lastError = "SIOCSIFMTU error @" + interfaceName;
            return false;
        }
    }

    return true;
}

