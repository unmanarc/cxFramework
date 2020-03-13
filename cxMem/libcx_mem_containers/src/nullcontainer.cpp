#include "nullcontainer.h"

NullContainer::NullContainer()
{
    bytes = 0;
}

bool NullContainer::streamTo(StreamableObject *, WRStatus &)
{
    return false;
}

WRStatus NullContainer::write(const void *, const size_t &count, WRStatus &)
{
    bytes+=count;
    return true;
}
