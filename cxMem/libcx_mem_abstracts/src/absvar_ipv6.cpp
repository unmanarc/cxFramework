#include "absvar_ipv6.h"
#include <arpa/inet.h>
#include <string.h>

AbsVar_IPV6::AbsVar_IPV6()
{
    memset(&value, 0, sizeof(value));
    setVarType(ABSTRACT_IPV6);
}

AbsVar_IPV6::~AbsVar_IPV6()
{
}

in6_addr AbsVar_IPV6::getValue()
{
    Lock_Mutex_RD lock(mutex);
    return value;
}

bool AbsVar_IPV6::setValue(const in6_addr &value)
{
    Lock_Mutex_RW lock(mutex);
    this->value.__in6_u = value.__in6_u;
    return true;
}

std::string AbsVar_IPV6::toString()
{
    in6_addr xvalue = getValue();
    char cIpSource[INET6_ADDRSTRLEN+2]="";
    inet_ntop(AF_INET6, &xvalue ,cIpSource, INET6_ADDRSTRLEN+2);
    return std::string(cIpSource);
}

bool AbsVar_IPV6::fromString(const std::string &value)
{
    if (value.empty())
    {
        in6_addr dfl;
        memset(&dfl,0,sizeof(dfl));
        setValue(dfl);
        return true;
    }

    in6_addr xvalue;
    bool r = inet_pton(AF_INET6, value.c_str(), &xvalue)==1;
    if (r == false) return false;
    setValue(xvalue);
    return true;
}

AbstractVar *AbsVar_IPV6::protectedCopy()
{
    AbsVar_IPV6 * var = new AbsVar_IPV6;
    if (var) *var = getValue();
    return var;
}
