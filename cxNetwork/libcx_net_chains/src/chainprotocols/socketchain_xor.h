#ifndef SOCKETCHAINXOR_H
#define SOCKETCHAINXOR_H

#include <cx_net_sockets/socket_base_stream.h>
#include "socketchainbase.h"

/**
 * @brief The SocketChainXOR class
 *        Proof of concept of socket transformation, don't use for security applications.
 */
class SocketChain_XOR : public Socket_Base_Stream, public SocketChainBase
{
public:
    SocketChain_XOR();

    // Overwritten functions:
    int partialRead(void * data, uint32_t datalen);
    int partialWrite(void * data, uint32_t datalen);

    // Private functions:
    char getXorByte() const;
    void setXorByte(char value);

protected:
    void * getThis() { return this; }

private:
    char * getXorCopy(void * data, uint32_t datalen);
    char xorByte;
};

#endif // SOCKETCHAINXOR_H
