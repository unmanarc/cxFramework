#ifndef ABSVAR_UINT8_H
#define ABSVAR_UINT8_H

#include "abstractvar.h"
#include <stdint.h>
#include <atomic>

class AbsVar_UINT8: public AbstractVar
{
public:
    AbsVar_UINT8();
    AbsVar_UINT8& operator=(uint8_t value)
    {
        setValue(value);
        return *this;
    }

    uint8_t getValue();
    bool setValue(uint8_t value);

    std::string toString() override;
    bool fromString(const std::string & value) override;
protected:
    AbstractVar * protectedCopy() override;
private:
    std::atomic<uint8_t> value;
};

#endif // ABSVAR_UINT8_H
