#include "base64std.h"

static const std::string b64Chars="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

std::string b64Encode(char const* buf, uint32_t count)
{
    unsigned char cont3[3],cont4[4];
    std::string encodedString;
    int x=0, y=0;

    while (count--)
    {
        cont3[x++]=*(buf++);
        if (x == 3)
        {
            cont4[0]=(cont3[0] & 0xfc) >> 2;
            cont4[1]=((cont3[0] & 0x03) << 4) + ((cont3[1] & 0xf0) >> 4);
            cont4[2]=((cont3[1] & 0x0f) << 2) + ((cont3[2] & 0xc0) >> 6);
            cont4[3]=cont3[2] & 0x3f;

            for(x=0; (x <4) ; x++)
            {
                encodedString += b64Chars[cont4[x]];
            }

            x=0;
        }
    }

    if (x)
    {
        for(y=x; y < 3; y++)
        {
            cont3[y]=0;
        }

        cont4[0]=(cont3[0] & 0xfc) >> 2;
        cont4[1]=((cont3[0] & 0x03) << 4) + ((cont3[1] & 0xf0) >> 4);
        cont4[2]=((cont3[1] & 0x0f) << 2) + ((cont3[2] & 0xc0) >> 6);
        cont4[3]=cont3[2] & 0x3f;

        for (y=0; (y < x + 1); y++)
        {
            encodedString += b64Chars[cont4[y]];
        }

        while((x++ < 3)!=0)
        {
            encodedString += '=';
        }
    }

    return encodedString;

}

std::string b64Decode(std::string const& sB64Buf)
{
    unsigned char cont4[4], cont3[3];
    std::string decodedString;
    size_t count=sB64Buf.size(), x=0, y=0;
    int bufPos=0;

    while (     count-- &&
                ( sB64Buf[bufPos] != '=')  &&
                (isalnum(sB64Buf[bufPos]) || (sB64Buf[bufPos] == '/') || (sB64Buf[bufPos] == '+'))
           )
    {
        cont4[x++]=sB64Buf[bufPos]; bufPos++;
        if (x==4)
        {
            for (x=0; x <4; x++)
            {
                cont4[x]=(unsigned char)b64Chars.find(cont4[x]);
            }

            cont3[0]=(cont4[0] << 2) + ((cont4[1] & 0x30) >> 4);
            cont3[1]=((cont4[1] & 0xf) << 4) + ((cont4[2] & 0x3c) >> 2);
            cont3[2]=((cont4[2] & 0x3) << 6) + cont4[3];

            for (x=0; (x < 3); x++)
            {
                decodedString += cont3[x];
            }
            x=0;
        }
    }

    if (x)
    {
        for (y=x; y <4; y++)
        {
            cont4[y]=0;
        }
        for (y=0; y <4; y++)
        {
            cont4[y]=(unsigned char)b64Chars.find(cont4[y]);
        }

        cont3[0]=(cont4[0] << 2) + ((cont4[1] & 0x30) >> 4);
        cont3[1]=((cont4[1] & 0xf) << 4) + ((cont4[2] & 0x3c) >> 2);
        cont3[2]=((cont4[2] & 0x3) << 6) + cont4[3];

        for (y=0; (y < x - 1); y++) decodedString += cont3[y];
    }

    return decodedString;
}
