#ifndef NANOTLS_H
#define NANOTLS_H

#include <openssl/err.h>
#include <openssl/ssl.h>

#include <string>
#include <list>

struct cipherBits
{
    cipherBits() {
        aSymBits = 0;
        symBits = 0;
    }
    int aSymBits, symBits;
};

class SharedTLS
{
public:
    SharedTLS();
    ~SharedTLS();

    void initHandle(bool validatePeer = false);
    bool initContext(bool serverMode = false);
    bool isInitialized();

    bool setCA(const std::string & file);
    bool setCRT(const std::string & file);
    bool setKEY(const std::string & file);

    bool setFileDescriptor(int sock);

    bool sslConnect();
    bool sslAccept();

    size_t partialRead(void * buffer, size_t len);
    size_t partialWrite(void * buffer, size_t len);

    std::list<std::string> getErrorsAndClear();

    std::string getCipherName();
    cipherBits getCipherBits();
    std::string getCipherVersion();
    bool validateConnection();

    std::string getPeerCommonName();
    std::string getPeerSubject();

private:
    void parseErrors();

    std::list<std::string> errors;

    STACK_OF(X509_NAME) *list;

    SSL *sslHandle;
    SSL_CTX *sslContext;
};

#endif // NANOTLS_H
