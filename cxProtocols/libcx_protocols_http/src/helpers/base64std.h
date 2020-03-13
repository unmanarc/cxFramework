#ifndef BASE64STD_H
#define BASE64STD_H

#include <string>

std::string b64Encode(const char *buf, uint32_t count);
std::string b64Decode(std::string const& sB64Buf);

#endif // BASE64STD_H
