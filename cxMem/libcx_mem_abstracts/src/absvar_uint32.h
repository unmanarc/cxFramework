#ifndef ABSVAR_UINT32_H
#define ABSVAR_UINT32_H

#include "abstractvar.h"
#include <stdint.h>
#include <atomic>

class AbsVar_UINT32: public AbstractVar
{
public:
    AbsVar_UINT32();
    AbsVar_UINT32& operator=(uint32_t value)
    {
        setValue(value);
        return *this;
    }

    uint32_t getValue();
    bool setValue(uint32_t value);

    std::string toString() override;
    bool fromString(const std::string & value) override;
protected:
    AbstractVar * protectedCopy() override;

private:
    std::atomic<uint32_t> value;
};

#endif // ABSVAR_UINT32_H
