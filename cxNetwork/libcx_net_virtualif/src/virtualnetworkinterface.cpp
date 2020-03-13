#include "virtualnetworkinterface.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#include <linux/if_tun.h>
//#include <linux/if.h>

#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#include <pwd.h>
#include <grp.h>

#include <cx_net_ifcfg/netifconfig.h>

VirtualNetworkInterface::VirtualNetworkInterface()
{
    MTU = 1500;
    fd = -1;
}

int VirtualNetworkInterface::start(bool configure)
{
    // Open the TUN/TAP device...
    if((fd = open("/dev/net/tun", O_RDWR)) < 0)
    {
        lastError = "/dev/net/tun error";
        return fd;
    }

    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));

    // Create the tun/tap interface.
    ifr.ifr_flags = IFF_TAP | IFF_NO_PI;

    if (interfaceName.c_str()[interfaceName.size()-1]>='0' && interfaceName.c_str()[interfaceName.size()-1]<='9')
    {
        snprintf(ifr.ifr_name, IFNAMSIZ, "%s",interfaceName.c_str() );
    }
    else
    {
        snprintf(ifr.ifr_name, IFNAMSIZ, "%s%%d",interfaceName.c_str() );
    }

    if(ioctl(fd, TUNSETIFF, (void*) &ifr) < 0)
    {
        lastError = "TUNSETIFF error";
        stop();
        return -1;
    }

    interfaceRealName = ifr.ifr_name;
    // Configure MTU and UP.

    if (configure)
    {
        NetIfConfig iface;
        if (iface.openInterface(interfaceRealName))
        {
            iface.setMTU(MTU);
            iface.setPromiscMode(true);
            iface.setUP(true);
            iface.apply();
        }
        else
        {
            lastError = iface.getLastError();
            stop();
            return -1;
        }
    }

    return fd;
}

/*
void
tuncfg(const char *dev, const char *dev_type, const char *dev_node, int persist_mode, const char *username, const char *groupname, const struct tuntap_options *options)
{
    struct tuntap *tt;

    ALLOC_OBJ(tt, struct tuntap);
    clear_tuntap(tt);
    tt->type = dev_type_enum(dev, dev_type);
    tt->options = *options;
    open_tun(dev, dev_type, dev_node, tt);
    if (ioctl(tt->fd, TUNSETPERSIST, persist_mode) < 0)
    {
        msg(M_ERR, "Cannot ioctl TUNSETPERSIST(%d) %s", persist_mode, dev);
    }
    if (username != NULL)
    {
        struct platform_state_user platform_state_user;

        if (!platform_user_get(username, &platform_state_user))
        {
            msg(M_ERR, "Cannot get user entry for %s", username);
        }
        else if (ioctl(tt->fd, TUNSETOWNER, platform_state_user.pw->pw_uid) < 0)
        {
            msg(M_ERR, "Cannot ioctl TUNSETOWNER(%s) %s", username, dev);
        }
    }
    if (groupname != NULL)
    {
        struct platform_state_group platform_state_group;

        if (!platform_group_get(groupname, &platform_state_group))
        {
            msg(M_ERR, "Cannot get group entry for %s", groupname);
        }
        else if (ioctl(tt->fd, TUNSETGROUP, platform_state_group.gr->gr_gid) < 0)
        {
            msg(M_ERR, "Cannot ioctl TUNSETOWNER(%s) %s", groupname, dev);
        }
    }
    close_tun(tt);
    msg(M_INFO, "Persist state set to: %s", (persist_mode ? "ON" : "OFF"));
}
*/


ssize_t VirtualNetworkInterface::writePacket(const void *packet, unsigned int len)
{
    std::unique_lock<std::mutex> lock(mutexWrite);
    return write(fd,packet,len);
}

ssize_t VirtualNetworkInterface::readPacket(void *packet, unsigned int len)
{
    return read(fd,packet,len);
}

void VirtualNetworkInterface::stop()
{
    if (fd>=0)
        close(fd);
    fd = -1;
}

void VirtualNetworkInterface::setInterfaceName(const std::string &value)
{
    interfaceName = value;
}

int VirtualNetworkInterface::getMTU() const
{
    return MTU;
}

void VirtualNetworkInterface::setMTU(const int &value)
{
    MTU = value;
}

bool VirtualNetworkInterface::setPersistentMode(bool mode)
{
    if (fd<0) return false;
    int iPersistent = mode?1:0;
    if (ioctl(fd, TUNSETPERSIST, iPersistent) < 0)
        return false;
    return true;
}

bool VirtualNetworkInterface::setOwner(const char *userName)
{
    if (fd<0) return false;
    char pwd_buf[4096];
    struct passwd pwd, *p_pwd;

    getpwnam_r(userName,&pwd,pwd_buf,sizeof(pwd_buf), &p_pwd);
    if (p_pwd)
    {
        if (ioctl(fd, TUNSETOWNER, p_pwd->pw_uid) < 0)
            return false;
        return true;
    }
    return false;
}

bool VirtualNetworkInterface::setGroup(const char *groupName)
{
    if (fd<0) return false;
    char grp_buf[4096];
    struct group grp, *p_grp;

    getgrnam_r(groupName,&grp,grp_buf,sizeof(grp_buf), &p_grp);
    if (p_grp)
    {
        if (ioctl(fd, TUNSETGROUP, p_grp->gr_gid) < 0)
            return false;
        return true;
    }
    return false;
}

std::string VirtualNetworkInterface::getInterfaceRealName() const
{
    return interfaceRealName;
}

std::string VirtualNetworkInterface::getLastError() const
{
    return lastError;
}
