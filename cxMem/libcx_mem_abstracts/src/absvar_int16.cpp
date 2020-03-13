#include "absvar_int16.h"
#include <stdexcept>      // std::invalid_argument

AbsVar_INT16::AbsVar_INT16()
{
    value = 0;
    setVarType(ABSTRACT_INT16);

}

int16_t AbsVar_INT16::getValue()
{
    return value;
}

bool AbsVar_INT16::setValue(int16_t value)
{
    this->value = value;
    return true;
}

std::string AbsVar_INT16::toString()
{
    return std::to_string(value);
}

bool AbsVar_INT16::fromString(const std::string &value)
{
    if (value.empty())
    {
        this->value = 0;
        return true;
    }

    size_t pos ;
    try
    {
        this->value = static_cast<int16_t>(std::stoi( value, &pos, 10 ));
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

AbstractVar *AbsVar_INT16::protectedCopy()
{
    AbsVar_INT16 * var = new AbsVar_INT16;
    if (var) *var = this->value;
    return var;
}
