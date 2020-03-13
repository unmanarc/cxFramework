#include "hlp_auth_strings.h"

#include <random>

std::string createRandomString(std::string::size_type length)
{
    char baseChars[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.";
    std::string randomStr;
    std::mt19937 rg{std::random_device{}()};
    std::uniform_int_distribution<std::string::size_type> pick(0, sizeof(baseChars)-2);
    randomStr.reserve(length);
    while(length--) randomStr += baseChars[pick(rg)];
    return randomStr;
}

void createRandomSalt(unsigned char * salt)
{
    std::mt19937 rg{std::random_device{}()};
    std::uniform_int_distribution<uint32_t> pick;
    *((uint32_t *)salt) = pick(rg);
}

bool mustBeEncoded(char c)
{
    if (c >= 'A' && c<= 'Z') return false;
    if (c >= 'a' && c<= 'z') return false;
    if (c >= '0' && c<= '9') return false;
    return true;
}

size_t calcExpandedStringSize(const std::string &str)
{
    size_t x = 0;
    for (size_t i=0; i<str.size();i++)
    {
        if ( mustBeEncoded(str.at(i)) ) x+=3;
        else x+=1;
    }
    return x;
}

char toHex(char value, char part)
{
    if (part == 1) value = value/0x10;
    else if (part == 2) value = value&0xF;
    if (value >= 0x0 && value <=0x9) return '0'+value;
    if (value >= 0xA && value <=0xF) return 'A'+value-0xA;
    return '0';
}

bool isHexChar(char v)
{
    return (v>='0' && v<='9') || (v>='A' && v<='F');
}

char hexToValue(char v)
{
    if (v>='0' && v<='9') return v-'0';
    if (v>='A' && v<='F') return v-'A'+10;
    if (v>='a' && v<='f') return v-'a'+10;
    return 0;
}

std::string toURLEncoded(const std::string &str)
{
    if (!str.size()) return "";

    size_t x=0;
    std::string out;
    out.resize(calcExpandedStringSize(str),' ');

    for (size_t i=0; i<str.size();i++)
    {
        if ( mustBeEncoded(str.at(i)) )
        {
            out[x++]='%';
            out[x++]=toHex(str.at(i), 1);
            out[x++]=toHex(str.at(i), 2);
        }
        else
        {
            out[x++] = str.at(i);
        }
    }
    return out;
}

std::string fromURLEncoded(const std::string &urlEncodedStr)
{
    std::string r;
    if (!urlEncodedStr.size()) return "";

    for (size_t i=0; i<urlEncodedStr.size();i++)
    {
        if ( urlEncodedStr[i] == '%' && i+3<=urlEncodedStr.size() && isHexChar(urlEncodedStr[i+1]) && isHexChar(urlEncodedStr[i+2]) )
        {
            char v = hexToValue(urlEncodedStr[i+1])*0x10 + hexToValue(urlEncodedStr[i+2]);
            r+=v;
            i+=2;
        }
        else
        {
            r+=urlEncodedStr[i];
        }
    }
    return r;
}

std::string strToHex(const unsigned char *data, size_t len)
{
    std::string r;
    for (size_t x = 0; x<len; x++)
    {
        char buf[4];
        sprintf(buf, "%02X", data[x]);
        r.append( buf );
    }
    return r;
}

void strFromHex(const std::string &hexValue, unsigned char *data, size_t maxlen)
{
    if ((hexValue.size()/2)<maxlen) maxlen=(hexValue.size()/2);
    for (size_t i=0;i<(maxlen*2);i+=2)
    {
        data[i/2] = hexToValue(hexValue.at(i))*0x10 + hexToValue(hexValue.at(i+1));
    }
}
