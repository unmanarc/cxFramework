#include "absvar_stringlist.h"

AbsVar_STRINGLIST::AbsVar_STRINGLIST()
{
    setVarType(ABSTRACT_STRINGLIST);
}

AbsVar_STRINGLIST::~AbsVar_STRINGLIST()
{
}

std::list<std::string> AbsVar_STRINGLIST::getValue()
{
    Lock_Mutex_RD lock(mutex);
    return value;
}

bool AbsVar_STRINGLIST::setValue(const std::list<std::string> &value)
{
    Lock_Mutex_RW lock(mutex);
    this->value = value;
    return true;
}

std::string AbsVar_STRINGLIST::toString()
{
    std::list<std::string> xvalue = getValue();
    // TODO:  use "" and escape seq CSV format.
    std::string r;
    bool first = true;
    for (const std::string & element : xvalue)
    {
        r += (!first? "," : "") + element;
        if (first) first = false;
    }
    return r;
}

bool AbsVar_STRINGLIST::fromString(const std::string &value)
{
    // TODO:  use "" and escape seq CSV format.
    std::list<std::string> strs;
    std::string::size_type curpos = 0, pos = std::string::npos;
    while (1)
    {
        pos = value.find_first_of(',',curpos);
        // Last word....
        std::string svalue;
        if (pos != std::string::npos)
        {
            svalue = value.substr(curpos, pos-curpos);
            strs.push_back(svalue);
            curpos=pos+1;
        }
        else
        {
            svalue = value.substr(curpos, std::string::npos);
            strs.push_back(svalue);
            break;
        }
    }
    return setValue(strs);
}

AbstractVar *AbsVar_STRINGLIST::protectedCopy()
{
    AbsVar_STRINGLIST * var = new AbsVar_STRINGLIST;
    if (var) *var = getValue();
    return var;
}
