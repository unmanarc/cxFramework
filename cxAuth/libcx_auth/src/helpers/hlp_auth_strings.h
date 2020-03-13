#ifndef STR_URL_CONVERTER_H
#define STR_URL_CONVERTER_H

#include <string>

void createRandomSalt(unsigned char *salt);
std::string createRandomString(std::string::size_type length);
std::string toURLEncoded(const std::string & str);
std::string fromURLEncoded(const std::string & urlEncodedStr);
std::string strToHex(const unsigned char *data, size_t len);
void strFromHex(const std::string & hexValue, unsigned char *data, size_t maxlen);

#endif // STR_URL_CONVERTER_H
