#ifndef JASYNCRPC_METHOD_H
#define JASYNCRPC_METHOD_H

#include <json/json.h>
#include <list>

struct sJSONMethod
{
    sJSONMethod()
    {
        mode = "method";
    }
    void fromJSON( const Json::Value & x )
    {
        if (!x["method"].isNull()) method = x["method"].asString();
        if (!x["mode"].isNull()) mode = x["mode"].asString();
        if (!x["payload"].isNull()) payload = x["payload"];
        if (!x["extraInfo"].isNull()) extraInfo = x["extraInfo"];
        if (!x["auth"].isNull() && x["auth"].size())
        {
            for (uint32_t y=0;x<x["auth"].size();y++)
            {
                auths.push_back(x["auth"][y]);
            }
        }
        if (!x["id"].isNull()) id = x["id"].asUInt64();
    }
    Json::Value toJSON()
    {
        Json::Value x;
        x["method"] = method;
        x["mode"] = mode;
        x["payload"] = payload;

        uint32_t y=0;
        for (const Json::Value & t : auths)
        {
            x["auth"][y++] = t;
        }

	x["id"] = Json::UInt64(id);
        return x;
    }

    uint64_t id;
    std::string method, mode;
    std::list<Json::Value> auths;
    Json::Value payload, extraInfo;
};


#endif // JASYNCRPC_METHOD_H
