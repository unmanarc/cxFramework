#include "absvar_bin.h"
#include <string.h>

AbsVar_BIN::AbsVar_BIN()
{
    setVarType(ABSTRACT_BIN);
}

AbsVar_BIN::~AbsVar_BIN()
{

}

sBinContainer *AbsVar_BIN::getValue()
{
    value.mutex.lock();
    return &value;
}

bool AbsVar_BIN::setValue(sBinContainer *value)
{
    Lock_Mutex_RW lock(this->value.mutex);
    this->value.ptr = new char[value->dataSize];
    if (!this->value.ptr) return false;
    this->value.dataSize = value->dataSize;
    memcpy(this->value.ptr, value->ptr, value->dataSize);
    return true;
}

std::string AbsVar_BIN::toString()
{
    Lock_Mutex_RD lock(this->value.mutex);
    std::string x( ((char *)value.ptr), value.dataSize);
    return x;
}

bool AbsVar_BIN::fromString(const std::string &value)
{
    Lock_Mutex_RW lock(this->value.mutex);
    this->value.ptr = new char[value.size()+1];
    if (!this->value.ptr) return false;
    this->value.ptr[value.size()] = 0;
    memcpy(this->value.ptr,value.c_str(),value.size());
    return true;
}

AbstractVar *AbsVar_BIN::protectedCopy()
{
    Lock_Mutex_RD lock(this->value.mutex);
    AbsVar_BIN * var = new AbsVar_BIN;
    if (!var->setValue(&(this->value)))
    {
        delete var;
        return nullptr;
    }
    return var;
}
