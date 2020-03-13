#include "absvar_ipv4.h"
#include <arpa/inet.h>

AbsVar_IPV4::AbsVar_IPV4()
{
    value.s_addr = 0;
    setVarType(ABSTRACT_IPV4);
}

AbsVar_IPV4::~AbsVar_IPV4()
{
}

in_addr AbsVar_IPV4::getValue()
{
    Lock_Mutex_RD lock(mutex);
    return value;
}

bool AbsVar_IPV4::setValue(const in_addr &value)
{
    Lock_Mutex_RW lock(mutex);
    this->value.s_addr = value.s_addr;
    return true;
}

std::string AbsVar_IPV4::toString()
{
    in_addr xvalue = getValue();
    char cIpSource[INET_ADDRSTRLEN+2]="";
    inet_ntop(AF_INET, &xvalue ,cIpSource, INET_ADDRSTRLEN+2);
    return std::string(cIpSource);
}

bool AbsVar_IPV4::fromString(const std::string &value)
{
    if (value.empty())
    {
        in_addr dfl;
        dfl.s_addr = 0;
        setValue(dfl);
        return true;
    }

    in_addr xvalue;
    bool r = inet_pton(AF_INET, value.c_str(), &xvalue)==1;
    if (r == false) return false;
    setValue(xvalue);
    return true;
}

AbstractVar *AbsVar_IPV4::protectedCopy()
{
    AbsVar_IPV4 * var = new AbsVar_IPV4;
    if (var) *var = getValue();
    return var;
}
