#include "socket_tls.h"

#include <cx_net_sockets/socket_base_stream.h>

#include <openssl/rand.h>
#include <openssl/err.h>

#include <iostream>
using namespace std;

Socket_TLS::Socket_TLS()
{
    isServer = false;
    // ssl empty, create a new one.
    sharedTLS.reset(new SharedTLS);
}

Socket_TLS::~Socket_TLS()
{
}

void Socket_TLS::prepareTLS()
{
    // Register the error strings for libcrypto & libssl
    SSL_load_error_strings ();
    ERR_load_crypto_strings();
    // Register the available ciphers and digests
    SSL_library_init ();
}

bool Socket_TLS::postConnectSubInitialization()
{
    if (sharedTLS->isInitialized()) return false; // already connected (don't connect again)

    isServer = false;

    if (!tlsInitContext())
    {
        sslErrors = sharedTLS->getErrorsAndClear();
        sharedTLS.reset(new SharedTLS);
        return false;
    }
    sharedTLS->initHandle();

    if (!sharedTLS->setFileDescriptor(getSocket()))
    {
        sslErrors = sharedTLS->getErrorsAndClear();
        sharedTLS.reset(new SharedTLS);
        return false;
    }
    else if (!(*sharedTLS).sslConnect())
    {
        sslErrors = sharedTLS->getErrorsAndClear();
        sharedTLS.reset(new SharedTLS);
        return false;
    }

    if (!sharedTLS->validateConnection())
    {
        sslErrors = sharedTLS->getErrorsAndClear();
        sharedTLS.reset(new SharedTLS);
        return false;
    }

    // connected!
    return true;
}

bool Socket_TLS::postAcceptSubInitialization()
{
    if (sharedTLS->isInitialized()) return false; // already connected (don't connect again)

    isServer = true;

    if (!tlsInitContext())
    {
        sslErrors = sharedTLS->getErrorsAndClear();
        sharedTLS.reset(new SharedTLS);
        return false;
    }

    // ssl empty, create a new one.
    sharedTLS->initHandle( ca_file.size()>0 );

    if (!sharedTLS->setFileDescriptor(getSocket()))
    {
        sslErrors = sharedTLS->getErrorsAndClear();
        sharedTLS.reset(new SharedTLS);
        return false;
    }
    else if (!(*sharedTLS).sslAccept())
    {
        sslErrors = sharedTLS->getErrorsAndClear();
        sharedTLS.reset(new SharedTLS);
        return false;
    }

    // connected!
    return true;
}

bool Socket_TLS::tlsInitContext()
{
    // create new SSL Context.
    if (!sharedTLS->initContext(isServer)) return false;

    if (!ca_file.empty() && !sharedTLS->setCA(ca_file))
    {
        return false;
    }
    if (!crt_file.empty() && !sharedTLS->setCRT(crt_file))
    {
        return false;
    }
    if (!key_file.empty() && !sharedTLS->setKEY(key_file))
    {
        return false;
    }

    return true;
}

std::list<std::string> Socket_TLS::getTLSErrorsAndClear()
{
    std::list<std::string> sslErrors2 = sharedTLS->getErrorsAndClear();
    for (std::string & i : sslErrors2) sslErrors.push_back(i);
    std::list<std::string> sslErrors3 = sslErrors;
    sslErrors.clear();
    return sslErrors3;
}

string Socket_TLS::getTLSPeerCN()
{
    if (!sharedTLS->isInitialized())
        return "";
    return sharedTLS->getPeerCommonName();
}

std::string Socket_TLS::getCertificateAuthorityPath() const
{
    return ca_file;
}

std::string Socket_TLS::getPrivateKeyPath() const
{
    return key_file;
}

std::string Socket_TLS::getPublicKeyPath() const
{
    return crt_file;
}

void Socket_TLS::setServerMode(bool value)
{
    isServer = value;
}

void Socket_TLS::setTLSContextMode(const SSL_MODE &value)
{
    sslMode = value;
}

bool Socket_TLS::setTLSCertificateAuthorityPath(const char *_ca_file)
{
    if (access(_ca_file,R_OK)) return false;
    ca_file = _ca_file;
    return true;
}

bool Socket_TLS::setTLSPublicKeyPath(const char *_crt_file)
{
    if (access(_crt_file,R_OK)) return false;
    crt_file = _crt_file;
    return true;
}

bool Socket_TLS::setTLSPrivateKeyPath(const char *_key_file)
{
    if (access(_key_file,R_OK)) return false;
    key_file = _key_file;
    return true;
}

string Socket_TLS::getCipherName()
{
    return sharedTLS->getCipherName();
}

cipherBits Socket_TLS::getCipherBits()
{
    return sharedTLS->getCipherBits();
}

string Socket_TLS::getProtocolVersionName()
{
    return sharedTLS->getCipherVersion();
}

Socket_Base_Stream * Socket_TLS::acceptConnection()
{
    Socket_Base_Stream * mainSock = Socket_TCP::acceptConnection();
    if (!mainSock) return nullptr;
    Socket_TLS * tlsSock = new Socket_TLS; // Convert to this thing...

    isServer = true;

    // Set current retrieved socket.
    tlsSock->setSocket(mainSock->getSocket());
    char remotePair[64];
    mainSock->getRemotePair(remotePair);
    tlsSock->setRemotePair(remotePair);
    tlsSock->setRemotePort(mainSock->getRemotePort());
    // detach, because tlsSock will rule the socket file descriptor now.
    mainSock->detachSocket();
    // now we should copy the context parameters:
    delete mainSock;

    if (!ca_file.empty()) tlsSock->setTLSCertificateAuthorityPath( ca_file.c_str() );
    if (!crt_file.empty()) tlsSock->setTLSPublicKeyPath( crt_file.c_str() );
    if (!key_file.empty()) tlsSock->setTLSPrivateKeyPath( key_file.c_str() );

    tlsSock->setTLSContextMode(sslMode);
    tlsSock->setServerMode(isServer);

    return tlsSock;
}

int Socket_TLS::partialRead(void *data, uint32_t datalen)
{
    return sharedTLS->partialRead(data,datalen);
}

int Socket_TLS::partialWrite(void *data, uint32_t datalen)
{
    return sharedTLS->partialWrite(data,datalen);
}
