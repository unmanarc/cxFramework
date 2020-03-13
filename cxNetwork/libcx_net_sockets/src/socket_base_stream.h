#ifndef SOCKET_BASE_STREAM_H_
#define SOCKET_BASE_STREAM_H_

#include "socket.h"
#include "stream_reader.h"
#include "stream_writer.h"
#include <utility>
#include <cx_mem_streams/streamableobject.h>

class Socket_Base_Stream : public StreamableObject, public Socket, public Stream_Reader, public Stream_Writer
{
public:
    Socket_Base_Stream();
    virtual ~Socket_Base_Stream()override;

    virtual void writeEOF(bool) override;

    /*
    uint64_t streamTo(StreamableObject * objDst, bool * readSucceed);
    size_t write(const void * buf, const size_t &count, bool * writeSucceed);*/

    bool streamTo(StreamableObject * out, WRStatus & wrsStat) override;
    WRStatus write(const void * buf, const size_t &count, WRStatus & wrStatUpd) override;

    /**
     * @brief GetSocketPair Create a Pair of interconnected sockets
     * @return pair of interconnected Socket_Base_Streams. (remember to delete them)
    */
    static std::pair<Socket_Base_Stream *, Socket_Base_Stream *> GetSocketPair();

    virtual bool isConnected() override;
    // This methods are virtual and should be implemented in sub-classes.
    // TODO: virtual redefinition?
    virtual bool listenOn(const uint16_t & port, const char * listenOnAddr, bool useIPv4, const int32_t & recvbuffer = 0, const int32_t &backlog = 10);
    virtual bool connectTo(const char * hostname, const uint16_t &port, const uint32_t &timeout) override;
    virtual Socket_Base_Stream * acceptConnection();

    /**
     * Virtual function for protocol initialization after the connection starts...
     * useful for SSL server, it runs in blocking mode and should be called apart to avoid tcp accept while block
     * @return returns true if was properly initialized.
     */
    virtual bool postAcceptSubInitialization();
    /**
     * Virtual function for protocol initialization after the connection starts (client-mode)...
     * useful for sub protocols initialization (eg. ssl), it runs in blocking mode.
     * @return returns true if was properly initialized.
     */
    virtual bool postConnectSubInitialization();

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Basic R/W options:
    /**
     * Write Null Terminated String on the socket
     * note: it writes the '\0' on the socket.
     * @param data null terminated string
     * @return true if the string was successfully sent
     */
    virtual bool writeBlock(const void * buf);
    /**
     * Write a data block on the socket
     * Send the data block in chunks until it ends or fail.
     * You can specify sizes bigger than 4k/8k, or megabytes (be careful with memory), and it will be fully sent in chunks.
     * @param data data block.
     * @param datalen data length in bytes
     * @return true if the data block was sucessfully sent.
     */
    virtual bool writeBlock(const void * data, const uint32_t & datalen) override;
    /**
     * Read a data block from the socket
     * Receive the data block in 4k chunks (or less) until it ends or fail.
     * You can specify sizes bigger than 4k/8k, or megabytes (be careful with memory), and it will be fully received in chunks.
     * You may want to specify read a timeout for your protocol.
     * @param data data block.
     * @param datalen data length in bytes
     * @return true if the data block was sucessfully received.
     */
    virtual bool readBlock(void * data, const uint32_t &datalen, uint32_t * bytesReceived = nullptr) override;

};

#endif /* SOCKET_BASE_STREAM_H_ */
