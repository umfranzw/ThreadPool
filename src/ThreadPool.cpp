#include "ThreadPool.h"

#include <iostream>

ThreadPool::ThreadPool(unsigned int maxThreads /*=0*/) : 
    m_maxThreads(maxThreads ? maxThreads : std::thread::hardware_concurrency()), 
    m_threads(m_maxThreads),
    m_shutdownPool(false)
{
    for (std::thread& thread : m_threads)
    {
        thread = std::thread(&ThreadPool::acquireWork, this);
    }
}

ThreadPool::~ThreadPool()
{
    {
        std::unique_lock<std::mutex> lock(m_mut);
        m_shutdownPool = true;
    }

    m_condVar.notify_all();

    for (auto& thread : m_threads)
    {
        if (thread.joinable())
            thread.join();
    }
}

void ThreadPool::acquireWork()
{
    bool done = false;
    while (!done)
    {
        std::packaged_task<void()> task;

        {
            std::unique_lock<std::mutex> lock(m_mut);
            m_condVar.wait(lock, [this]() {
                return m_shutdownPool || !m_queue.empty();
            });

            done = m_shutdownPool;
            if (!done)
            {
                task = std::move(m_queue.front());
                m_queue.pop_front();
            }
        }

        if (!done)
            task();
    }
}

unsigned int ThreadPool::getMaxThreads()
{
    return m_maxThreads;
}
