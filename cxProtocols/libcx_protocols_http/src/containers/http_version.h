#ifndef HTTP_VERSION_H
#define HTTP_VERSION_H

#include <string>
#include <stdint.h>

class HTTP_Version
{
public:
    HTTP_Version();
    void parseVersion(const std::string & version);
    std::string getHTTPVersionString();

    uint16_t getVersionMinor() const;
    void setVersionMinor(const uint16_t &value);

    void upgradeMinorVersion(const uint16_t &value);

    uint16_t getVersionMajor() const;
    void setVersionMajor(const uint16_t &value);

private:
    /**
     * @brief versionMinor - Version Minor
     */
    uint16_t versionMinor;
    /**
     * @brief versionMajor - Version Major
     */
    uint16_t versionMajor;

};

#endif // HTTP_VERSION_H
