#include "absvar_double.h"
#include <stdexcept>      // std::invalid_argument

AbsVar_DOUBLE::AbsVar_DOUBLE()
{
    value = 0;
    setVarType(ABSTRACT_DOUBLE);

}

double AbsVar_DOUBLE::getValue()
{
    return value;
}

void AbsVar_DOUBLE::setValue(const double &value)
{
    this->value = value;
}

std::string AbsVar_DOUBLE::toString()
{
    return std::to_string(value);
}

bool AbsVar_DOUBLE::fromString(const std::string &value)
{
    try
    {
        this->value = std::stod( value ) ;
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

AbstractVar *AbsVar_DOUBLE::protectedCopy()
{
    AbsVar_DOUBLE * var = new AbsVar_DOUBLE;
    if (var) *var = this->value;
    return var;
}
