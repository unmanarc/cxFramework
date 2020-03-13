#ifndef JSON_STREAMABLEOBJECT_H
#define JSON_STREAMABLEOBJECT_H

#include <cx_mem_streams/streamableobject.h>
#include <json/json.h>

class JSON_StreamableObject : public StreamableObject
{
public:
    JSON_StreamableObject();

    static std::string jsonToString( const Json::Value & value );

    bool streamTo(StreamableObject * out, WRStatus & wrStatUpd) override;
    WRStatus write(const void * buf, const size_t &count, WRStatus & wrStatUpd)  override;
    void writeEOF(bool) override;

    void clear();

    std::string getString();

    Json::Value * processValue();
    Json::Value * getValue();

    void setValue(const Json::Value & value);

    void setMaxSize(const uint64_t &value);

private:
    uint64_t maxSize;
    std::string strValue;
    Json::Value root;
};

#endif // JSON_STREAMABLEOBJECT_H
