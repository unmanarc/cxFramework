#include "socket_unix.h"

#include <sys/types.h>
#include <unistd.h>
#include <string.h>

#include <sys/un.h>
#include <sys/socket.h>

Socket_UNIX::Socket_UNIX()
{
}


bool Socket_UNIX::listenOn(const uint16_t & port, const char * listenOnAddr)
{
    if (isActive()) closeSocket(); // close first

   // use the addr as path.
   sockaddr_un server_address;
   int         server_len;

   unlink(listenOnAddr);

   setSocket(socket(AF_UNIX, SOCK_STREAM, 0));
   if (!isActive())
   {
      lastError = "socket() failed";
      return false;
   }

   server_address.sun_family = AF_UNIX;
   strncpy(server_address.sun_path, listenOnAddr, sizeof(server_address.sun_path)-1);
   server_len = sizeof(server_address);

   if (bind(getSocket(),(struct sockaddr *)&server_address,server_len) < 0)
   {
       lastError = "bind() failed";
       closeSocket();
       return false;
   }
   if (listen(getSocket(), 10) < 0)
   {
       lastError = "bind() failed";
       closeSocket();
       return false;
   }

   listenMode = true;
   return true;
}

bool Socket_UNIX::connectTo(const char * hostname, const uint16_t &port, const uint32_t & timeout)
{
    if (isActive()) closeSocket(); // close first

    int         len;
    sockaddr_un address;

    setSocket( socket(AF_UNIX, SOCK_STREAM, 0) );
    if (!isActive())
    {
        lastError = "socket() failed";
        return false;
    }

    address.sun_family = AF_UNIX;
    strcpy(address.sun_path, hostname);
    len = sizeof(address);

    // Set the timeout here.
    setReadTimeout(timeout);

    if(connect(getSocket(), (sockaddr*)&address, len) == -1)
    {
        lastError = "socket() failed";
        return false;
    }

    return true;
}

Socket_Base_Stream * Socket_UNIX::acceptConnection()
{
    int sdconn;

    Socket_Base_Stream * cursocket = nullptr;

    if ((sdconn = accept(getSocket(), nullptr, nullptr)) >= 0)
    {
        cursocket = new Socket_Base_Stream;
        // Set the proper socket-
        cursocket->setSocket(sdconn);
    }
    else
    {
        // Establish the error.
        lastError = "accept() failed";
    }

    // return the socket class.
    return cursocket;
}
