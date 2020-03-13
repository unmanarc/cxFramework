#include "mutex_map_element.h"

Mutex_Map_Element::Mutex_Map_Element()
{
    xMapFinished = false;
}

Mutex_Map_Element::~Mutex_Map_Element()
{

}

void Mutex_Map_Element::stopReaders()
{
    xMapFinished = true;

    // Send signal stop for this element...
    stopSignal();
}

void Mutex_Map_Element::stopSignal()
{
    // NOT IMPLEMENTED HERE.
}
