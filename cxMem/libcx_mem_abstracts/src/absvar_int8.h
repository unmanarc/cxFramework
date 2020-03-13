#ifndef ABSVAR_INT8_H
#define ABSVAR_INT8_H

#include "abstractvar.h"
#include <stdint.h>
#include <atomic>

class AbsVar_INT8: public AbstractVar
{
public:
    AbsVar_INT8();
    AbsVar_INT8& operator=(int8_t value)
    {
        setValue(value);
        return *this;
    }

    int8_t getValue();
    bool setValue(int8_t value);

    std::string toString() override;
    bool fromString(const std::string & value) override;
protected:
    AbstractVar * protectedCopy() override;

private:
    std::atomic<int8_t> value;
};

#endif // ABSVAR_INT8_H
