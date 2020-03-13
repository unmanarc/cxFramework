#include "absvar_uint64.h"
#include <stdexcept>      // std::invalid_argument

AbsVar_UINT64::AbsVar_UINT64()
{
    value = 0;
    setVarType(ABSTRACT_UINT64);
}

uint64_t AbsVar_UINT64::getValue()
{
    return value;
}

bool AbsVar_UINT64::setValue(const uint64_t &value)
{
    this->value = value;
    return true;
}

std::string AbsVar_UINT64::toString()
{
    return std::to_string(value);
}

bool AbsVar_UINT64::fromString(const std::string &value)
{
    if (value.empty())
    {
        this->value = 0;
        return true;
    }

    size_t pos ;
    try
    {
        this->value = std::stoull( value, &pos, 10 ) ;
        return true;
    }
    catch( std::invalid_argument * )
    {
        return false;
    }
    catch ( std::out_of_range * )
    {
        return false;
    }
}

AbstractVar *AbsVar_UINT64::protectedCopy()
{
    AbsVar_UINT64 * var = new AbsVar_UINT64;
    if (var) *var = this->value;
    return var;
}
