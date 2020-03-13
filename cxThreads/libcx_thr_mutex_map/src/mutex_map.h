#ifndef XMAP_H
#define XMAP_H

#include <mutex>
#include <condition_variable>
#include <map>
#include <set>
#include <atomic>

#include <unistd.h>

#include "mutex_map_element.h"

struct sMapElement
{
    sMapElement()
    {
        readers = 0;
        rdElement = nullptr;
    }
    Mutex_Map_Element *rdElement;
    std::atomic<uint32_t> readers;
    std::condition_variable cond_zeroReaders;
};

template <class T>
class Mutex_Map
{
public:
    Mutex_Map() {}

    std::set<T> getKeys();

    bool addElement( const T & key, Mutex_Map_Element * element );
    Mutex_Map_Element * openElement(const T & key);
    bool closeElement(const T & key);
    bool destroyElement(const T & key);

    void waitForEmpty();

private:
    std::map<T,sMapElement> xMap;
    std::condition_variable cond_zeroMaps;
    std::mutex mutex_xMap;
};


template<class T>
std::set<T> Mutex_Map<T>::getKeys()
{
    std::unique_lock<std::mutex> lock(mutex_xMap);
    std::set<T> ret;
    for (const auto & i : xMap) ret.insert(i.first);
    return ret;
}

template<class T>
bool Mutex_Map<T>::addElement(const T &key, Mutex_Map_Element *element)
{
    std::unique_lock<std::mutex> lock(mutex_xMap);
    if (xMap.find(key) == xMap.end())
    {
        xMap[key].rdElement = element;
        return true;
    }
    return false;
}

// FAST
template<class T>
Mutex_Map_Element *Mutex_Map<T>::openElement(const T &key)
{
    std::unique_lock<std::mutex> lock(mutex_xMap);
    if (xMap.find(key) != xMap.end() && xMap[key].rdElement)
    {
        xMap[key].readers++;
        return xMap[key].rdElement;;
    }
    return nullptr;
}

// FAST
template<class T>
bool Mutex_Map<T>::closeElement(const T &key)
{
    std::unique_lock<std::mutex> lock(mutex_xMap);

    if (xMap.find(key) != xMap.end())
    {
        if (xMap[key].readers==0)
            throw std::runtime_error("Invalid close on Mutex MAP");

        xMap[key].readers--;
        // if no more readers... emit the signal to notify it:
        if (xMap[key].readers == 0)
        {
            xMap[key].cond_zeroReaders.notify_one();
        }
        return true;
    }
    return false;
}

template<class T>
bool Mutex_Map<T>::destroyElement(const T &key)
{
    std::unique_lock<std::mutex> lock(mutex_xMap);

    if (    xMap.find(key) != xMap.end()
            && xMap[key].rdElement != nullptr )
    {
        // No more open readers and destroy element.. (inaccesible for openElement and for destroyElement)
        Mutex_Map_Element * delElement = xMap[key].rdElement;
        xMap[key].rdElement = nullptr;

        for (;xMap[key].readers != 0;)
        {
            delElement->stopReaders();
            // unlock and retake the lock until signal is emited.
            xMap[key].cond_zeroReaders.wait(lock);
        }

        // Now is time to delete and remove.
        delete delElement;
        xMap.erase(key);
        if (xMap.empty())
            cond_zeroMaps.notify_one();
        return true;
    }
    return false;
}

template<class T>
void Mutex_Map<T>::waitForEmpty()
{
    std::unique_lock<std::mutex> lock(mutex_xMap);
    if (xMap.empty()) return;
    cond_zeroMaps.wait(lock);
}

#endif // XMAP_H
