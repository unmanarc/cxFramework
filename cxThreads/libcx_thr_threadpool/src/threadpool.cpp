#include "threadpool.h"

#include <iostream>

template <class RandomAccessIterator> void safe_random_shuffle (RandomAccessIterator first, RandomAccessIterator last, std::size_t hash)
{
    std::minstd_rand0 gen(hash);
    typename std::iterator_traits<RandomAccessIterator>::difference_type i, n;
    n = (last-first);
    for (i=n-1; i>0; --i)
    {
        std::uniform_int_distribution<> dis(0, static_cast<int>(i));
        std::swap (first[i],first[dis(gen)]);
    }
}

void taskProcessor(ThreadPool * tp)
{
    for (sTask task = tp->popTask();
         !task.isNull();
         task = tp->popTask())
    {
        //std::cout << "ejecutando " << task.data << std::endl << std::flush;
        task.task(task.data);
    }
}

ThreadPool::ThreadPool(uint32_t threadsCount, uint32_t taskQueues)
{
    std::random_device rd;
    lRand.seed(rd());

    setTasksByQueueLimit(100);

    terminate = false;
    queuedElements = 0;
    this->threadsCount = threadsCount;
    for (size_t i =0; i<taskQueues;i++)
    {
        queues[i].init = true;
    }
}

ThreadPool::~ThreadPool()
{
    stop();
    for (uint32_t i =0; i<threadsCount;i++) threads[i].join();
}

void ThreadPool::start()
{
    for (size_t i =0; i<threadsCount;i++)
    {
        threads[i] = std::thread(taskProcessor, this);
    }
}

void ThreadPool::stop()
{
    std::unique_lock<std::mutex> lk(mutexQueues);
    terminate = true;
    lk.unlock();
    cond_insertedElement.notify_all();
}

bool ThreadPool::pushTask(void (*task)(void *), void *data, uint32_t timeoutMS,  const float &priority, const std::string &key)
{
    size_t currentQueue = getRandomQueueByKey(key,priority);

    // TODO: put to best place first
    std::unique_lock<std::mutex> lk(mutexQueues);

    // Don't insert on termination...
    if (terminate)
        return false;

    // Check if the queue is up the limit
    while ( queues[currentQueue].tasks.size() > tasksByQueueLimit  )
    {
        if (timeoutMS == static_cast<uint32_t>(-1))
        {
            queues[currentQueue].cond_removedElement.wait(lk);
        }
        else
        {
            if (queues[currentQueue].cond_removedElement.wait_for(lk, std::chrono::milliseconds(timeoutMS)) == std::cv_status::timeout)
            {
                return false;
            }
        }
    }

    // Now is not full, insert it.
    sTask toInsert;
    toInsert.data = data;
    toInsert.task = task;
    queues[currentQueue].tasks.push( toInsert );

    // Notify that there is one element in one of the lists...
    lk.unlock();
    cond_insertedElement.notify_one();
    return true;
}

sTask ThreadPool::popTask()
{
    // lock and wait for an incomming task
    std::unique_lock<std::mutex> lk(mutexQueues);

    sTaskQueue * tq = getRandomTaskQueueWithElements();
    while ( tq == nullptr )
    {
        // No available elements...
        cond_insertedElement.wait(lk);

        // On termination, empty queue means exit
        if ( terminate && (tq=getRandomTaskQueueWithElements()) == nullptr)
        {
            sTask r;
            return r;
        }
        else if (!terminate)
            tq = getRandomTaskQueueWithElements();
    }

    sTask r= tq->tasks.front();
    tq->tasks.pop();

    // Notify!
    lk.unlock();
    tq->cond_removedElement.notify_one();
    return r;
}

sTaskQueue *ThreadPool::getRandomTaskQueueWithElements()
{
    std::vector<size_t> fullVector;
    // Randomize the full vector
    for (size_t i=0; i<queues.size(); ++i) fullVector.push_back(i);
    std::uniform_int_distribution<size_t> dis;
    mutexRandom.lock();
    safe_random_shuffle(fullVector.begin(), fullVector.end(),static_cast<size_t>(dis(lRand)));
    mutexRandom.unlock();

    // Iterate full Vector...
    for ( size_t i : fullVector )
    {
        if (!queues[i].tasks.empty())
        {
            return &(queues[i]);
        }
    }
    return nullptr;
}

size_t ThreadPool::getRandomQueueByKey(const std::string &key, const float &priority)
{
    size_t x;
    std::vector<size_t> reducedVector;
    std::vector<size_t> fullVector;

    // Convert priority in vector elements...
    size_t elements = static_cast<size_t>(queues.size()*priority);
    if (elements==0) elements = 1;
    if (elements>queues.size()) elements = queues.size();

    // Randomize the full vector using the hash of key
    for (size_t i=0; i<queues.size(); ++i) fullVector.push_back(i);
    safe_random_shuffle(fullVector.begin(), fullVector.end(), hash_fn(key));

    // Copy the first n-elements (based on priority)
    for (size_t i=0;i<elements;i++) reducedVector.push_back(fullVector[i]);

    // Get random element from the reduced vector:
    std::uniform_int_distribution<> dis(0, static_cast<int>(elements-1));
    mutexRandom.lock();
    x = reducedVector.at( static_cast<size_t>(dis(lRand)) );
    mutexRandom.unlock();

    return x;
}

uint32_t ThreadPool::getTasksByQueueLimit() const
{
    return tasksByQueueLimit;
}

void ThreadPool::setTasksByQueueLimit(const uint32_t &value)
{
    tasksByQueueLimit = value;
    for (auto & i : queues) i.second.cond_removedElement.notify_all();
}
