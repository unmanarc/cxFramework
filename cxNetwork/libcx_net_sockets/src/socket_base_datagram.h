#ifndef SOCKET_BASE_DATAGRAM_H_
#define SOCKET_BASE_DATAGRAM_H_

#include "socket.h"
#include <memory>
#include <string.h>

#ifdef _WIN32
#include <Winsock2.h>
#else
#include <netinet/in.h>
#endif

struct DatagramBlock
{
    DatagramBlock()
    {
        data = nullptr;
        datalen = -1;
    }
    ~DatagramBlock()
    {
        this->free();
    }
    void free()
    {
        if (data) delete [] data;
    }
    void copy(void * _data, int dlen)
    {
        if (dlen>0 && dlen<1024*1024) // MAX: 1Mb.
        {
            this->free();
            data = new unsigned char[dlen];
            memcpy(data,_data,dlen);
        }
    }
    struct sockaddr addr;
    unsigned char * data;
    int datalen;
};


class Socket_Base_Datagram : public Socket
{
public:
	Socket_Base_Datagram();
	virtual ~Socket_Base_Datagram();

    // Datagram Specific Functions.
    virtual std::shared_ptr<DatagramBlock> readDatagramBlock() = 0;

    // Socket specific functions:
    virtual bool isConnected() = 0;
    virtual bool listenOn(const uint16_t & port,const char * listenOnAddr) = 0;
    virtual bool connectTo(const char * hostname, const uint16_t & port, const uint32_t &timeout) = 0;
    virtual bool writeBlock(const void * data, const uint32_t &datalen) = 0;
    virtual bool readBlock(void * data, const uint32_t & datalen) = 0;
};

#endif /* SOCKET_BASE_DATAGRAM_H_ */
