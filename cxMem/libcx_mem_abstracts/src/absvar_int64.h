#ifndef ABSVAR_INT64_H
#define ABSVAR_INT64_H

#include "abstractvar.h"
#include <stdint.h>
#include <atomic>

class AbsVar_INT64: public AbstractVar
{
public:
    AbsVar_INT64();
    AbsVar_INT64& operator=(const int64_t &value)
    {
        setValue(value);
        return *this;
    }

    int64_t getValue();
    bool setValue(const int64_t &value);

    std::string toString() override;
    bool fromString(const std::string & value) override;
protected:
    AbstractVar * protectedCopy() override;

private:
    std::atomic<int64_t> value;
};

#endif // ABSVAR_INT64_H
