#ifndef ABSVAR_STRINGLIST_H
#define ABSVAR_STRINGLIST_H

#include "abstractvar.h"
#include <list>
#include <cx_thr_mutex/lock_mutex_shared.h>

class AbsVar_STRINGLIST: public AbstractVar
{
public:
    AbsVar_STRINGLIST();
    ~AbsVar_STRINGLIST() override;
    AbsVar_STRINGLIST& operator=(const std::list<std::string> & value)
    {
        setValue(value);
        return *this;
    }
    std::list<std::string> getValue();
    bool setValue(const std::list<std::string> &value);

    std::string toString() override;
    bool fromString(const std::string & value) override;
protected:
    AbstractVar * protectedCopy() override;

private:
    std::list<std::string> value;
    Mutex_RW mutex;
};

#endif // ABSVAR_STRINGLIST_H
