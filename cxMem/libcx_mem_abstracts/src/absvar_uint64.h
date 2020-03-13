#ifndef ABSVAR_UINT64_H
#define ABSVAR_UINT64_H

#include "abstractvar.h"
#include <stdint.h>
#include <atomic>

class AbsVar_UINT64: public AbstractVar
{
public:
    AbsVar_UINT64();
    AbsVar_UINT64& operator=(uint64_t value)
    {
        setValue(value);
        return *this;
    }

    uint64_t getValue();
    bool setValue(const uint64_t &value);

    std::string toString() override;
    bool fromString(const std::string & value) override;
protected:
    AbstractVar * protectedCopy() override;

private:
    std::atomic<uint64_t> value;
};

#endif // ABSVAR_UINT64_H
