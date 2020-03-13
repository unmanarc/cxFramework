#ifndef TS_QUEUE_H
#define TS_QUEUE_H

#include <queue>
#include <atomic>
#include <mutex>
#include <condition_variable>

/**
 * @brief The Thread Safe Queue class
 */
template <class T>
class TS_Queue
{
public:
    TS_Queue()
    {
        maxItems = &localMaxItems;
        localMaxItems = 0xFFFFFFFF;
    }
    ~TS_Queue()
    {
        std::unique_lock<std::mutex> lock(mQueue);
        while (_queue.size())
        {
            delete _queue.front();
            _queue.pop();
        }
    }
    /**
     * @brief setMaxItemsAsExternalPointer Set Max Queue Items as External Pointer
     * @param maxItems Max Queue Items Pointer.
     */
    void setMaxItemsAsExternalPointer(std::atomic<size_t> * maxItems);
    /**
     * @brief setMaxItems Set the value of the current max items object
     * @param maxItems value
     */
    void setMaxItems(const size_t & maxItems);
    /**
     * @brief getMaxItems Get max items allowed
     * @return integer with max items allowed.
     */
    size_t getMaxItems();
    /**
     * @brief push Push a new item
     * @param item item to be pushed to the queue
     * @param tmout_msecs Timeout in milliseconds if the queue is full (default: 100 seconds)
     * @return
     */
    bool push(T * item ,const uint32_t & tmout_msecs = 100000);
    /**
     * @brief pop Pop the item, you should delete/remove it after using "destroyItem" function
     * @param tmout_msecs Timeout in milliseconds if the queue is empty (default: 100 seconds)
     * @return the first item in the queue
     */
    T * pop(const uint32_t & tmout_msecs = 100000);
    /**
     * @brief size Current Queue Size
     * @return size in size_t format...
     */
    size_t size();

private:
    std::atomic<size_t> * maxItems;
    std::atomic<size_t> localMaxItems;
    std::mutex mQueue;
    std::condition_variable condNotEmpty, condNotFull;
    std::queue<T *> _queue;
};



template<class T>
void TS_Queue<T>::setMaxItemsAsExternalPointer(std::atomic<size_t> *maxItems)
{
    this->maxItems = maxItems;
}

template<class T>
void TS_Queue<T>::setMaxItems(const size_t &maxItems)
{
    (*this->maxItems) = maxItems;
}

template<class T>
size_t TS_Queue<T>::getMaxItems()
{
    return *maxItems;
}

template<class T>
bool TS_Queue<T>::push(T *item, const uint32_t &tmout_msecs)
{
    std::unique_lock<std::mutex> lock(mQueue);
    while (_queue.size()>=*maxItems)
    {
        if (condNotFull.wait_for(lock, std::chrono::milliseconds(tmout_msecs)) == std::cv_status::timeout)
            return false;
    }
    _queue.push(item);
    lock.unlock();
    condNotEmpty.notify_one();
    return true;
}

template<class T>
T *TS_Queue<T>::pop(const uint32_t &tmout_msecs)
{
    std::unique_lock<std::mutex> lock(mQueue);
    while (_queue.empty())
    {
        if (condNotEmpty.wait_for(lock, std::chrono::milliseconds(tmout_msecs)) == std::cv_status::timeout)
            return nullptr;
    }
    T * r = _queue.front();
    _queue.pop();
    lock.unlock();
    condNotFull.notify_one();
    return r;
}

template<class T>
size_t TS_Queue<T>::size()
{
    std::unique_lock<std::mutex> lock(mQueue);
    return _queue.size();
}



#endif // TS_QUEUE_H
