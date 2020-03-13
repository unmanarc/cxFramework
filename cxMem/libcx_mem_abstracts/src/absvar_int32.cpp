#include "absvar_int32.h"
#include <stdexcept>      // std::invalid_argument

AbsVar_INT32::AbsVar_INT32()
{
    value = 0;
    setVarType(ABSTRACT_INT32);
}

int32_t AbsVar_INT32::getValue()
{
    return value;
}

bool AbsVar_INT32::setValue(int32_t value)
{
    this->value = value;
    return true;
}

std::string AbsVar_INT32::toString()
{
    return std::to_string(value);

}

bool AbsVar_INT32::fromString(const std::string &value)
{
    if (value.empty())
    {
        this->value = 0;
        return true;
    }

    size_t pos ;
    try
    {
        this->value = std::stoi( value, &pos, 10 ) ;
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

AbstractVar *AbsVar_INT32::protectedCopy()
{
    AbsVar_INT32 * var = new AbsVar_INT32;
    if (var) *var = this->value;
    return var;
}
