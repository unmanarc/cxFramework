#include "absvar_bool.h"
#include <stdexcept>      // std::invalid_argument

AbsVar_BOOL::AbsVar_BOOL()
{
    value = false;
    setVarType(ABSTRACT_BOOL);

}

bool AbsVar_BOOL::getValue()
{
    return value;
}

bool AbsVar_BOOL::setValue(bool value)
{
    this->value = value;
    return true;
}

std::string AbsVar_BOOL::toString()
{
    return value?"true":"false";
}

bool AbsVar_BOOL::fromString(const std::string &value)
{
    if (value == "true" || value == "TRUE" || value == "1" || value == "t" || value == "T") this->value = true;
    else this->value = false;
    return true;
}

AbstractVar *AbsVar_BOOL::protectedCopy()
{
    AbsVar_BOOL * var = new AbsVar_BOOL;
    if (var) *var = this->value;
    return var;
}
