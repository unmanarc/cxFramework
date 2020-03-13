#ifndef ABSVAR_BIN_H
#define ABSVAR_BIN_H

#include "abstractvar.h"
#include <cx_thr_mutex/lock_mutex_shared.h>

struct sBinContainer {
    sBinContainer()
    {
        ptr = nullptr;
    }
    ~sBinContainer()
    {
        if (ptr) delete [] ptr;
    }

    char * ptr;
    uint32_t dataSize;
    Mutex_RW mutex;
};

class AbsVar_BIN : public AbstractVar
{
public:
    AbsVar_BIN();
    virtual ~AbsVar_BIN() override;

    /**
     * @brief getValue Get container memory position
     * @return container memory position and mutex locked (you have to unlock it).
     */
    sBinContainer *getValue();
    bool setValue(sBinContainer *value);

    std::string toString() override;
    bool fromString(const std::string & value) override;

protected:
    AbstractVar * protectedCopy() override;

private:
    sBinContainer value;
};

#endif // ABSVAR_BIN_H
