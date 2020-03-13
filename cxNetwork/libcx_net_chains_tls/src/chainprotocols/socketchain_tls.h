#ifndef SOCKETCHAIN_TLS_H
#define SOCKETCHAIN_TLS_H

#include <cx_net_tls/socket_tls.h>
#include <cx_net_chains/socketchainbase.h>

class SocketChain_TLS : public Socket_TLS, public SocketChainBase
{
public:
    SocketChain_TLS();

protected:
    void * getThis() override { return this; }

};

#endif // SOCKETCHAIN_TLS_H
