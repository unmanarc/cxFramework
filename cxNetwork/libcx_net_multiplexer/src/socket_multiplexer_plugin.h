#ifndef SOCKET_MULTIPLEXER_PLUGINS_H
#define SOCKET_MULTIPLEXER_PLUGINS_H

#include <string>
#include <json/json.h>

class Socket_Mutiplexer_Plugin
{
public:
    Socket_Mutiplexer_Plugin();
    virtual ~Socket_Mutiplexer_Plugin();
    std::string getPluginId() const;
    void setMultiplexerPtr(void *value);
    /**
     * @brief processJSON16 Process JSON (up to 64Kb in size)
     * @param value json value.
     */
    virtual void processJSON16(const Json::Value & value) {}
    /**
     * @brief processData Process data block.
     * @param data
     * @param len
     */
    virtual void processData(const char * data, const unsigned int &len) {}
    /**
     * @brief eventOnBackLineConnect Called before processing the protocol.
     */
    virtual void eventOnMultiplexedSocketConnect() {}
    /**
     * @brief eventOnBackLineFinish Called after processing the protocol (socket may be alive or not)
     */
    virtual void eventOnMultiplexedSocketFinalization() {}
    /**
     * @brief eventOnLineConnect when trying to connect.
     * @return true if connection is authorized, false if disapproved (one pluggin is sufficient to disapprove)
     */
    virtual bool eventOnLineConnect() { return true; }

protected:
    void setPluginId(const std::string &value);
    bool sendJSON16(const Json::Value & value);

    void * multiplexerPtr;
private:
    std::string pluginId;
};

#endif // SOCKET_MULTIPLEXER_PLUGINS_H
