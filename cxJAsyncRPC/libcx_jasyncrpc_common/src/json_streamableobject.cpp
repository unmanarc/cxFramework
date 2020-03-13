#include "json_streamableobject.h"
#include <limits>
#include <iostream>

JSON_StreamableObject::JSON_StreamableObject()
{
    // No max size (original...)
    maxSize = std::numeric_limits<uint64_t>::max();
}

std::string JSON_StreamableObject::jsonToString(const Json::Value &value)
{
    Json::FastWriter fastWriter;
    std::string xstrValue = fastWriter.write(value);
    if (!xstrValue.empty() && xstrValue[xstrValue.length()-1] == '\n')
    {
        xstrValue.erase(xstrValue.length()-1);
    }
    return xstrValue;
}


bool JSON_StreamableObject::streamTo(StreamableObject *out, WRStatus &wrStatUpd)
{
    WRStatus cur;
    strValue = jsonToString(root);
    return (cur = out->writeFullStream(strValue.c_str(), strValue.size(), wrStatUpd)).succeed;
}

WRStatus JSON_StreamableObject::write(const void *buf, const size_t &count, WRStatus &wrStatUpd)
{
    // TODO: how to report that the max size has been exceeded.
    WRStatus cur;

    // ...
    if ( strValue.size()+count > maxSize ) cur.bytesWritten = maxSize-strValue.size();
    else                                   cur.bytesWritten = count;

    if (cur.bytesWritten)
        strValue += std::string(((const char *)buf),cur.bytesWritten); // Copy...
    else
        wrStatUpd.finish = cur.finish = true;

    // Append...
    wrStatUpd.bytesWritten+=cur.bytesWritten;

    return cur;
}

void JSON_StreamableObject::writeEOF(bool)
{
    processValue();
}

void JSON_StreamableObject::clear()
{
    Json::Value x;
    root = x;
    //root.clear();
    strValue.clear();
}

std::string JSON_StreamableObject::getString()
{
    Json::FastWriter fastWriter;
    return fastWriter.write(root);
}

Json::Value *JSON_StreamableObject::processValue()
{
    Json::Reader reader;
    bool parsingSuccessful = reader.parse( strValue, root );
    if ( !parsingSuccessful )
        return nullptr;
    return &root;
}

Json::Value *JSON_StreamableObject::getValue()
{
  //  std::cout << "getting value" << std::endl << std::flush;
    return &root;
}

void JSON_StreamableObject::setValue(const Json::Value &value)
{
//    std::cout << "setting value to " << value.toStyledString() << std::endl << std::flush;
    root=value;
}

void JSON_StreamableObject::setMaxSize(const uint64_t &value)
{
    maxSize = value;
}
