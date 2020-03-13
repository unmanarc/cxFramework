#include "absvar_string.h"

AbsVar_STRING::AbsVar_STRING()
{
    setVarType(ABSTRACT_STRING);
}

AbsVar_STRING::~AbsVar_STRING()
{
}

std::string AbsVar_STRING::getValue()
{
    Lock_Mutex_RD lock(mutex);
    return value;
}

bool AbsVar_STRING::setValue(const std::string &value)
{
    return fromString(value);
}

std::string AbsVar_STRING::toString()
{
    return getValue();
}

bool AbsVar_STRING::fromString(const std::string &value)
{
    Lock_Mutex_RW lock(mutex);
    this->value = value;
    return true;
}

AbstractVar *AbsVar_STRING::protectedCopy()
{
    Lock_Mutex_RD lock(mutex);
    AbsVar_STRING * var = new AbsVar_STRING;
    if (var) *var = this->value;
    return var;
}
