#ifndef ABSVAR_INT16_H
#define ABSVAR_INT16_H

#include "abstractvar.h"
#include <stdint.h>
#include <atomic>

class AbsVar_INT16: public AbstractVar
{
public:
    AbsVar_INT16();
    AbsVar_INT16& operator=(int16_t value)
    {
        setValue(value);
        return *this;
    }

    int16_t getValue();
    bool setValue(int16_t value);

    std::string toString() override;
    bool fromString(const std::string & value) override;
protected:
    AbstractVar * protectedCopy() override;

private:
    std::atomic<int16_t> value;
};

#endif // ABSVAR_INT16_H
