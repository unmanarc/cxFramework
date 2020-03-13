#ifndef ABSTRACTVAR_H
#define ABSTRACTVAR_H

#include <string>

enum AVarType {
    ABSTRACT_BOOL,
    ABSTRACT_INT8,
    ABSTRACT_INT16,
    ABSTRACT_INT32,
    ABSTRACT_INT64,
    ABSTRACT_UINT8,
    ABSTRACT_UINT16,
    ABSTRACT_UINT32,
    ABSTRACT_UINT64,
    ABSTRACT_DOUBLE,
    ABSTRACT_BIN,
    ABSTRACT_STRING,
    ABSTRACT_STRINGLIST,
    ABSTRACT_IPV4,
    ABSTRACT_IPV6
};

class AbstractVar
{
public:
    AbstractVar();
    AbstractVar * copy();
    virtual ~AbstractVar();

    static AbstractVar * makeAbstractVar(AVarType type, const std::string & defValue = "");

    virtual std::string toString() = 0;
    virtual bool fromString(const std::string & value) = 0;

    // VAR TYPE:
    AVarType getVarType() const;
    void setVarType(const AVarType &value);

protected:
    virtual AbstractVar * protectedCopy() = 0;

private:
    AVarType varType;
};

#endif // ABSTRACTVAR_H
