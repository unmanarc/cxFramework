#ifndef ABSVAR_IPV4_H
#define ABSVAR_IPV4_H

#include "abstractvar.h"
#include <netinet/in.h>
#include <cx_thr_mutex/lock_mutex_shared.h>

class AbsVar_IPV4: public AbstractVar
{
public:
    AbsVar_IPV4();
    ~AbsVar_IPV4() override;
    AbsVar_IPV4& operator=(const in_addr & value)
    {
        setValue(value);
        return *this;
    }
    in_addr getValue();
    bool setValue(const in_addr & value);

    std::string toString() override;
    bool fromString(const std::string & value) override;
protected:
    AbstractVar * protectedCopy() override;

private:
    in_addr value;
    Mutex_RW mutex;
};

#endif // ABSVAR_IPV4_H
