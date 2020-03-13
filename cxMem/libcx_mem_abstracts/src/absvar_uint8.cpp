#include "absvar_uint8.h"
#include <stdexcept>      // std::invalid_argument

AbsVar_UINT8::AbsVar_UINT8()
{
    value = 0;
    setVarType(ABSTRACT_UINT8);
}

uint8_t AbsVar_UINT8::getValue()
{
    return value;
}

bool AbsVar_UINT8::setValue(uint8_t value)
{
    this->value = value;
    return true;
}

std::string AbsVar_UINT8::toString()
{
    return std::to_string(value);
}

bool AbsVar_UINT8::fromString(const std::string &value)
{
    if (value.empty())
    {
        this->value = 0;
        return true;
    }

    size_t pos ;
    try
    {
        this->value = static_cast<uint8_t>(std::stoul( value, &pos, 10 ));
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

AbstractVar *AbsVar_UINT8::protectedCopy()
{
    AbsVar_UINT8 * var = new AbsVar_UINT8;
    if (var) *var = this->value;
    return var;
}
