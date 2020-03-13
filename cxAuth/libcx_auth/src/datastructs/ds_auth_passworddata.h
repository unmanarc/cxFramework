#ifndef IAUTH_PASSWORDDATA_H
#define IAUTH_PASSWORDDATA_H

#include <json/json.h>
#include <limits>
#include <string.h>
#include "ds_auth_passmodes.h"
#include "hlp_auth_strings.h"

struct sPasswordBasicData
{
    sPasswordBasicData()
    {
        expiration = 0;
        forceExpiration = false;
        passwordMode = PASS_MODE_PLAIN;
        memset(ssalt,0,4);
    }

    bool isPasswordExpired()
    {
        return expiration<time(nullptr);
    }

    PasswordModes passwordMode;
    unsigned char ssalt[4];
    time_t expiration;
    bool forceExpiration;

    char align[7];
};


struct sPasswordData
{
    sPasswordData()
    {
        gAuthSteps = 0; // means current.
        forceExpiration = false;
        passwordMode = PASS_MODE_PLAIN;
        expiration = std::numeric_limits<time_t>::max();
        memset(ssalt,0xFF,4);
    }

    sPasswordBasicData getBasicData()
    {
        sPasswordBasicData B;
        B.expiration = expiration;
        B.forceExpiration = forceExpiration;
        B.passwordMode = passwordMode;
        memcpy(B.ssalt, ssalt ,4);
        return B;
    }

    Json::Value getJson() const
    {
        Json::Value j;
        j["mode"] = static_cast<uint32_t>(passwordMode);
        j["hash"] = hash;
        j["salt"] = strToHex(ssalt,4);
        j["expiration"] = static_cast<uint32_t>(expiration);
        j["forceExpiration"] = forceExpiration;
        j["gAuthSteps"] = gAuthSteps;
        return j;
    }

    bool setJson(const Json::Value & j)
    {
        passwordMode = static_cast<PasswordModes>(j["mode"].asUInt());
        expiration= j["expiration"].asUInt();
        strFromHex(j["salt"].asString(),ssalt,4);
        hash = j["hash"].asString();
        forceExpiration = j["forceExpiration"].asBool();
        gAuthSteps = j["gAuthSteps"].asUInt();
        return true;
    }

    time_t getExpiration() const
    {
        return expiration;
    }

    void setExpiration(const time_t &value)
    {
        expiration = value;
    }

    bool isPasswordExpired() const
    {
        return expiration<time(nullptr) || forceExpiration;
    }

    uint32_t gAuthSteps;
    bool forceExpiration;
    PasswordModes passwordMode;
    time_t expiration;
    std::string hash;
    unsigned char ssalt[4];
};


#endif // IAUTH_PASSWORDDATA_H
