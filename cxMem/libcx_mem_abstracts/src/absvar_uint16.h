#ifndef ABSVAR_UINT16_H
#define ABSVAR_UINT16_H

#include "abstractvar.h"
#include <stdint.h>
#include <atomic>

class AbsVar_UINT16: public AbstractVar
{
public:
    AbsVar_UINT16();
    AbsVar_UINT16& operator=(uint16_t value)
    {
        setValue(value);
        return *this;
    }

    uint16_t getValue();
    bool setValue(uint16_t value);

    std::string toString() override;
    bool fromString(const std::string & value) override;
protected:
    AbstractVar * protectedCopy() override;

private:
    std::atomic<uint16_t> value;
};

#endif // ABSVAR_UINT16_H
