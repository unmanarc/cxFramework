#include "abstractvar.h"

#include "absvar_bin.h"
#include "absvar_bool.h"
#include "absvar_double.h"
#include "absvar_int8.h"
#include "absvar_int16.h"
#include "absvar_int32.h"
#include "absvar_int64.h"
#include "absvar_ipv4.h"
#include "absvar_ipv6.h"
#include "absvar_string.h"
#include "absvar_stringlist.h"
#include "absvar_uint8.h"
#include "absvar_uint16.h"
#include "absvar_uint32.h"
#include "absvar_uint64.h"

AbstractVar::AbstractVar()
{

}

AbstractVar *AbstractVar::copy()
{
    AbstractVar * var = protectedCopy();
    if (var) var->setVarType(this->getVarType());
    return var;
}

AbstractVar::~AbstractVar()
{

}

AbstractVar *AbstractVar::makeAbstractVar(AVarType type, const std::string &defValue)
{
    AbstractVar * v = nullptr;
    switch (type)
    {
    case ABSTRACT_BOOL:
        v=new AbsVar_BOOL;
        break;
    case ABSTRACT_INT8:
        v=new AbsVar_INT8;
        break;
    case ABSTRACT_INT16:
        v=new AbsVar_INT16;
        break;
    case ABSTRACT_INT32:
        v=new AbsVar_INT32;
        break;
    case ABSTRACT_INT64:
        v=new AbsVar_INT64;
        break;
    case ABSTRACT_UINT8:
        v=new AbsVar_UINT8;
        break;
    case ABSTRACT_UINT16:
        v=new AbsVar_UINT16;
        break;
    case ABSTRACT_UINT32:
        v=new AbsVar_UINT32;
        break;
    case ABSTRACT_UINT64:
        v=new AbsVar_UINT64;
        break;
    case ABSTRACT_DOUBLE:
        v=new AbsVar_DOUBLE;
        break;
    case ABSTRACT_BIN:
        v=new AbsVar_BIN;
        break;
    case ABSTRACT_STRING:
        v=new AbsVar_STRING;
        break;
    case ABSTRACT_STRINGLIST:
        v=new AbsVar_STRINGLIST;
        break;
    case ABSTRACT_IPV4:
        v=new AbsVar_IPV4;
        break;
    case ABSTRACT_IPV6:
        v=new AbsVar_IPV6;
        break;
    default:
        break;
    }

    if (v) v->fromString(defValue);

    return v;
}

AVarType AbstractVar::getVarType() const
{
    return varType;
}

void AbstractVar::setVarType(const AVarType &value)
{
    varType = value;
}
