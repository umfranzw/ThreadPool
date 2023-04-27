#ifndef _THREADPOOL_H_
#define _THREADPOOL_H_

#include <vector>
#include <thread>
#include <future>
#include <type_traits>
#include <queue>
#include <functional>
#include <condition_variable>
#include <mutex>

class ThreadPool
{
public:
    ThreadPool(unsigned int maxThreads=0);
    ~ThreadPool();

    unsigned int getMaxThreads();

    template<typename F, typename... As> // function, args
    std::future< std::result_of_t<F(As&&...)> > enqueueTask(F&& fcn, As&&... args)
    {
        using R = std::result_of_t<F(As&&...)>;
        
        std::packaged_task<R()> task(
            // Use std::bind to create a callable object the encapsulates F and A.
            // This callable object accepts no arguments, and returns a value of type R: the result of calling F with A.
            // This means that the packaged task is of type R().
            // We can store a packaged task of type R() in a queue of type void().
            std::bind(
                std::forward<F>(fcn),
                std::forward<As>(args)...
            )
        );
        std::future<R> result(task.get_future());
        m_queue.emplace_back(std::move(task));

        return result;
    }

private:
    void acquireWork();

    unsigned int m_maxThreads;
    std::vector<std::thread> m_threads;
    std::deque<std::packaged_task<void()>> m_queue;
    std::condition_variable m_condVar;
    std::mutex m_mut;
    bool m_shutdownPool;
};

#endif