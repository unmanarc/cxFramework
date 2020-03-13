#ifndef ABSVAR_DOUBLE_H
#define ABSVAR_DOUBLE_H
#include "abstractvar.h"

#include <atomic>

class AbsVar_DOUBLE: public AbstractVar
{
public:
    AbsVar_DOUBLE();
    AbsVar_DOUBLE& operator=(double value)
    {
        setValue(value);
        return *this;
    }

    double getValue();
    void setValue(const double & value);

    std::string toString() override;
    bool fromString(const std::string & value) override;
protected:
    AbstractVar * protectedCopy() override;

private:
    std::atomic<double> value;
};

#endif // ABSVAR_DOUBLE_H
