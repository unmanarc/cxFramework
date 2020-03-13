#ifndef NETIFCONFIG_H
#define NETIFCONFIG_H

#include <string>

#include <netinet/in.h>
#include <net/if.h>

#include <linux/if_ether.h>

class NetIfConfig
{
public:
    NetIfConfig();
    ~NetIfConfig();

    bool openInterface(const std::string & _ifaceName);

    // Getters:
    int getMTU();
    ethhdr getEthernetAddress();
    std::string getLastError() const;

    // Setters:
    void setIPv4Address(const in_addr &_address,const in_addr &_netmask);
    void setMTU(int _mtu);
    void setPromiscMode(bool state=true);
    void setUP(bool state=true);

    // Apply:
    bool apply();

private:
    struct ifreq ifr;
    struct ifreq ifr1_host;
    struct ifreq ifr1_netmask;

    bool address_setted;

    std::string interfaceName;
    std::string lastError;
    int fd,MTU;

};

#endif // NETIFCONFIG_H
