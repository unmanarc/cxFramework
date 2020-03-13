#include "absvar_int8.h"
#include <stdexcept>      // std::invalid_argument

AbsVar_INT8::AbsVar_INT8()
{
    value = 0;
    setVarType(ABSTRACT_INT8);
}

int8_t AbsVar_INT8::getValue()
{
    return value;
}

bool AbsVar_INT8::setValue(int8_t value)
{
    this->value = value;
    return true;
}

std::string AbsVar_INT8::toString()
{
    return std::to_string(value);
}

bool AbsVar_INT8::fromString(const std::string &value)
{
    if (value.empty())
    {
        this->value = 0;
        return true;
    }
    size_t pos ;
    try
    {
        this->value = static_cast<int8_t>(std::stol( value, &pos, 10 ));
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

AbstractVar *AbsVar_INT8::protectedCopy()
{
    AbsVar_INT8 * var = new AbsVar_INT8;
    if (var) *var = this->value;
    return var;
}
