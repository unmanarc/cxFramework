#include "socket_base_stream.h"

#ifndef _WIN32
#include <sys/socket.h>
#else
#include <winsock2.h>
#include <Ws2tcpip.h>
#include "socket_tcp.h"
#endif
#include <string.h>
#include <unistd.h>

Socket_Base_Stream::Socket_Base_Stream()
{
}

Socket_Base_Stream::~Socket_Base_Stream()
{
}

void Socket_Base_Stream::writeEOF(bool)
{
    shutdownSocket(SHUT_RDWR);
}

bool Socket_Base_Stream::streamTo(StreamableObject *out, WRStatus &wrsStat)
{
    char data[8192];
    WRStatus cur;
    for (;;)
    {
        int r = partialRead(data,sizeof(data));
        switch (r)
        {
        case -1: // ERR.
            out->writeEOF(false);
            return false;
        case 0: // EOF.
            out->writeEOF(true);
            return true;
        default:
            if (!(cur=out->writeFullStream(data,r,wrsStat)).succeed || cur.finish)
            {
                if (!cur.succeed)
                {
                    out->writeEOF(false);
                    return false;
                }
                else
                {
                    out->writeEOF(true);
                    return true;
                }
            }
        break;
        }
    }
}

WRStatus Socket_Base_Stream::write(const void *buf, const size_t &count, WRStatus &wrStat)
{
    WRStatus cur;
    // TODO: report the right amount of data copied...
    bool r = writeBlock(buf,count);
    if (!r)
        wrStat.succeed=cur.succeed=setFailedWriteState();
    else
    {
        cur.bytesWritten+=count;
        wrStat.bytesWritten+=count;
    }
    return cur;
}

std::pair<Socket_Base_Stream *,Socket_Base_Stream *> Socket_Base_Stream::GetSocketPair()
{
    int sockets[2];
    std::pair<Socket_Base_Stream *,Socket_Base_Stream *> p;

    p.first = nullptr;
    p.second = nullptr;

#ifndef _WIN32
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockets) < 0)
    {
        // ERROR:...
    }
    else
    {
        p.first = new Socket_Base_Stream();
        p.second = new Socket_Base_Stream();

        p.first->setSocket(sockets[0]);
        p.second->setSocket(sockets[1]);
    }
#else
    // Emulate via TCP. (EXPERIMENTAL)

    Socket_TCP * llsock = new Socket_TCP, * lsock, * rsock = new Socket_TCP;
    llsock->listenOn(0,"127.0.0.1",true);
    rsock->connectTo("127.0.0.1",lsock->getPort());
    lsock = llsock->acceptConnection();
    llsock->closeSocket();
    delete llsock;

    p.first = lsock;
    p.second = rsock;
#endif
    return p;
}

bool Socket_Base_Stream::writeBlock(const void *data)
{
    return writeBlock(data,strlen(((const char *)data)));
}

bool Socket_Base_Stream::writeBlock(const void *data, const uint32_t &datalen)
{
   // if (!isActive()) return false;

    int32_t sent_bytes = 0;
    int32_t left_to_send = datalen;

    // Send the raw data.
    // datalen-left_to_send is the _size_ of the data already sent.
    while (left_to_send && (sent_bytes = partialWrite((char *) data + (datalen - left_to_send), left_to_send>4096?4096:left_to_send)) <= left_to_send)
    {
        if (sent_bytes == -1)
        {
            // Error sending data. (returns false.)
            shutdownSocket();
            return false;
        }
        // Substract the data that was already sent from the count.
        else
            left_to_send -= sent_bytes;
    }

    // Failed to achieve sending the contect on 5 attempts
    if (left_to_send != 0)
    {
        // left_to_send must always return 0 bytes. otherwise here we have an error (return false)
        shutdownSocket();
        return false;
    }
    return true;
}

Socket_Base_Stream * Socket_Base_Stream::acceptConnection()
{
    return nullptr;
}

bool Socket_Base_Stream::postAcceptSubInitialization()
{
    return true;
}

bool Socket_Base_Stream::postConnectSubInitialization()
{
    return true;
}

bool Socket_Base_Stream::readBlock(void *data, const uint32_t &datalen, uint32_t * bytesReceived)
{
    if (bytesReceived) *bytesReceived = 0;

    if (!isActive())
    {
        return false;
    }

    int total_recv_bytes = 0;
    int local_recv_bytes = 0;

    if (datalen==0) return true;

    // Try to receive the maximum amount of data left.
    while ( (datalen - total_recv_bytes)>0 // there are bytes to read.
            && (local_recv_bytes = partialRead(((char *) data) + total_recv_bytes, datalen - total_recv_bytes)) >0 // receive bytes. if error, will return with -1.
            )
    {
        // Count the data received.
        total_recv_bytes += local_recv_bytes;
    }

    if ((unsigned int)total_recv_bytes<datalen)
    {
        if (total_recv_bytes==0) return false;
        if (bytesReceived) *bytesReceived = total_recv_bytes;
        return true;
    }

    if (bytesReceived) *bytesReceived = datalen;

    // Otherwise... return true.
    return true;
}

bool Socket_Base_Stream::isConnected()
{
    if (!isActive()) return false;

    struct sockaddr peer;
    socklen_t peer_len;
    peer_len = sizeof(peer);
    if (getpeername(getSocket(), &peer, &peer_len) == -1)
    {
        closeSocket();
        return false;
    }
    return true;
}

bool Socket_Base_Stream::listenOn(const uint16_t &, const char *, bool , const int32_t &, const int32_t &)
{
    return false;
}

bool Socket_Base_Stream::connectTo(const char* , const uint16_t &, const uint32_t &)
{
	return false;
}
