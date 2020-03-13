#ifndef VIRTUALNETWORKINTERFACE_H
#define VIRTUALNETWORKINTERFACE_H

#include <string>
#include <mutex>
#include <stdint.h>

class VirtualNetworkInterface
{
public:
    VirtualNetworkInterface();

    // Start/Stop
    int start(bool configure=true);
    void stop();

    // Configuration
    void setInterfaceName(const std::string &value);
    void setMTU(const int &value);
    bool setPersistentMode(bool mode);
    bool setOwner(const char * userName);
    bool setGroup(const char * groupName);

    // Operation
    ssize_t writePacket(const void *packet, unsigned int len);
    ssize_t readPacket(void *packet, unsigned int len);

    // Status:
    int getMTU() const;
    std::string getInterfaceRealName() const;
    std::string getLastError() const;

private:
    std::mutex mutexWrite;
    std::string lastError;
    std::string interfaceName, interfaceRealName;
    int MTU;
    int fd;
};

#endif // VIRTUALNETWORKINTERFACE_H
