#ifndef SOCKET_TCP_SSL_H
#define SOCKET_TCP_SSL_H

#include <cx_net_sockets/socket_tcp.h>
#include <unistd.h>
#include <memory>
#include <string>
#include <list>

#include "sharedtls.h"

enum SSL_MODE {
    SSL_MODE_TLS_12
};

/**
 * TCP Socket Class
 */
class Socket_TLS : public Socket_TCP {
public:
	/**
	 * Class constructor.
	 */
	Socket_TLS();
	/**
	 * Class destructor.
	 */
	virtual ~Socket_TLS();
    /**
     * TLS server function for protocol initialization , it runs in blocking mode and should be called apart to avoid tcp accept while block
     * @return returns true if was properly initialized.
     */
    bool postAcceptSubInitialization();
    /**
     * Accept a new TCP connection on a listening socket.
     * @return returns a socket with the new established tcp connection.
     */
    Socket_Base_Stream *acceptConnection();
    /**
     * Read a data block from the TLS socket
     * Receive the data block in only one command (without chunks).
     * note that this haves some limitations. some systems can only receive 4k at time.
     * You may want to manage the chunks by yourself.
     * @param data data block.
     * @param datalen data length in bytes
     * @return return the number of bytes read by the socket, zero for end of file and -1 for error.
     */
    virtual int partialRead(void * data, uint32_t datalen);
    /**
     * Write a data block to the TLS socket
     * note that this haves some limitations. some systems can only send 4k at time.
     * You may want to manage the chunks by yourself.
     * @param data data block.
     * @param datalen data length in bytes
     * @return return the number of bytes read by the socket, zero for end of file and -1 for error.
     */
    virtual int partialWrite(void * data, uint32_t datalen);

    /////////////////////////
    // SSL functions:
    /**
     * Call this when program starts.
     */
    static void prepareTLS();
    /**
     * SSL protocol set.
     */
    void setTLSContextMode(const SSL_MODE &value);
    /**
     * Set TLS certificate authority chain file.
     * @return true if succeed
     */
    bool setTLSCertificateAuthorityPath(const char * _ca_file);
    /**
     * Set TLS Local certificate file.
     * @return true if succeed
     */
    bool setTLSPublicKeyPath(const char * _crt_file);
    /**
     * Set TLS Local certificate file.
     * @return true if succeed
     */
    bool setTLSPrivateKeyPath(const char * _key_file);
    /**
     * @brief getCertificateAuthorityPath
     * @return Certificate Authority Path
     */
    std::string getCertificateAuthorityPath()const;
    /**
     * @brief getPrivateKeyPath
     * @return private key path
     */
    std::string getPrivateKeyPath()const;
    /**
     * @brief getPublicKeyPath Get public key path
     * @return public key path
     */
    std::string getPublicKeyPath()const;
    /**
     * @brief getCipherName Get current cipher used.
     * @return current cipher
     */
    std::string getCipherName();
    /**
     * @brief getCipherBits Get current cipher bits used.
     * @return bits used
     */
    cipherBits getCipherBits();
    /**
     * @brief getProtocolVersionName Get protocol version name
     * @return cipher version string
     */
    std::string getProtocolVersionName();
    /**
     * @brief setServer Mark this socket as server (useful to determine if works as server or client)
     * @param value true for server.
     */
    void setServerMode(bool value);
   /**
     * @brief getTLSErrorsAndClear Get TLS/SSL Errors
     * @return List of SSL Errors.
     */
    std::list<std::string> getTLSErrorsAndClear();
    /**
     * @brief getTLSPeerCommonName Get TLS Peer Common Name
     * @return
     */
    std::string getTLSPeerCN();

protected:
    /**
     * function for TLS client protocol initialization after the connection starts (client-mode)...
     * @return returns true if was properly initialized.
     */
    bool postConnectSubInitialization();

private:
    bool tlsInitContext();

    /**
     * @brief ssl
     * this parameter is a shared pointer of ssl connection (copy will reference it.)
     */
    std::shared_ptr<SharedTLS> sharedTLS;

    std::string crt_file,key_file,ca_file;

    std::list<std::string> sslErrors;

    SSL_MODE sslMode;
    bool isServer;
};

#endif // SOCKET_TCP_SSL_H
