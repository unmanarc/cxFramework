#include "absvar_uint16.h"
#include <stdexcept>      // std::invalid_argument

AbsVar_UINT16::AbsVar_UINT16()
{
    value = 0;
    setVarType(ABSTRACT_UINT16);
}

uint16_t AbsVar_UINT16::getValue()
{
    return value;
}

bool AbsVar_UINT16::setValue(uint16_t value)
{
    this->value = value;
    return true;
}

std::string AbsVar_UINT16::toString()
{
    return std::to_string(value);
}

bool AbsVar_UINT16::fromString(const std::string &value)
{
    if (value.empty())
    {
        this->value = 0;
        return true;
    }


    size_t pos ;
    try
    {
        this->value = static_cast<uint16_t>(std::stoul( value, &pos, 10 ));
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

AbstractVar *AbsVar_UINT16::protectedCopy()
{
    AbsVar_UINT16 * var = new AbsVar_UINT16;
    if (var) *var = this->value;
    return var;
}
