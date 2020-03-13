#ifndef ABSVAR_BOOL_H
#define ABSVAR_BOOL_H

#include "abstractvar.h"
#include <stdint.h>
#include <atomic>

class AbsVar_BOOL: public AbstractVar
{
public:
    AbsVar_BOOL();
    AbsVar_BOOL& operator=(bool value)
    {
        setValue(value);
        return *this;
    }

    bool getValue();
    bool setValue(bool value);

    std::string toString() override;
    bool fromString(const std::string & value) override;
protected:
    AbstractVar * protectedCopy() override;
private:
    std::atomic<bool> value;
};

#endif // ABSVAR_BOOL_H
