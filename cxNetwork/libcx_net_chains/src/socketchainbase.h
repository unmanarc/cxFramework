#ifndef SOCKETCHAINBASE_H
#define SOCKETCHAINBASE_H

#include <utility>
#include <cx_net_sockets/socket_base_stream.h>

class SocketChainBase
{
public:
    SocketChainBase();
    virtual ~SocketChainBase();

    virtual bool isEndPoint();
    std::pair<Socket_Base_Stream *, Socket_Base_Stream*> makeSocketChainPair();
    bool isServerMode() const;
    void setServerMode(bool value);

protected:
    virtual void * getThis() = 0;

private:
    bool serverMode;
};

#endif // SOCKETCHAINBASE_H
