#ifndef ABSVAR_STRING_H
#define ABSVAR_STRING_H

#include "abstractvar.h"
#include <cx_thr_mutex/lock_mutex_shared.h>

class AbsVar_STRING: public AbstractVar
{
public:
    AbsVar_STRING();
    ~AbsVar_STRING() override;
    AbsVar_STRING& operator=(const std::string & value)
    {
        setValue(value);
        return *this;
    }
    std::string getValue();
    bool setValue(const std::string &value);

    std::string toString() override;
    bool fromString(const std::string & value) override;
protected:
    AbstractVar * protectedCopy() override;

private:
    std::string value;
    Mutex_RW mutex;
};

#endif // ABSVAR_STRING_H
