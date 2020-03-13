#ifndef ABSVAR_INT32_H
#define ABSVAR_INT32_H

#include "abstractvar.h"
#include <stdint.h>
#include <atomic>

class AbsVar_INT32: public AbstractVar
{
public:
    AbsVar_INT32();
    AbsVar_INT32& operator=(int32_t value)
    {
        setValue(value);
        return *this;
    }

    int32_t getValue();
    bool setValue(int32_t value);

    std::string toString() override;
    bool fromString(const std::string & value) override;
protected:
    AbstractVar * protectedCopy() override;

private:
    std::atomic<int32_t> value;
};

#endif // ABSVAR_INT32_H
