#ifndef ABSVAR_IPV6_H
#define ABSVAR_IPV6_H

#include "abstractvar.h"
#include <netinet/in.h>
#include <cx_thr_mutex/lock_mutex_shared.h>

class AbsVar_IPV6: public AbstractVar
{
public:
    AbsVar_IPV6();
    ~AbsVar_IPV6() override;
    AbsVar_IPV6& operator=(const in6_addr & value)
    {
        setValue(value);
        return *this;
    }

    in6_addr getValue();
    bool setValue(const in6_addr & value);

    std::string toString() override;
    bool fromString(const std::string & value) override;
protected:
    AbstractVar * protectedCopy() override;

private:
    in6_addr value;
    Mutex_RW mutex;
};

#endif // ABSVAR_IPV6_H
