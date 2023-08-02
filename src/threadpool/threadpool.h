#ifndef THREADPOOL_H
#define THREADPOOL_H

/*参考:https://github.com/progschj/ThreadPool*/

#include <vector>
#include <thread>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <future>

#include <glog/logging.h>

class ThreadPool {
private:
    ThreadPool();

public:
    static ThreadPool* getInstance();

    template<typename Functor, typename... ArgsType>
    auto enqueue(Functor&& f, ArgsType&&... args) {
        using return_type = typename std::result_of<Functor(ArgsType...)>::type;

        std::shared_ptr<std::packaged_task<return_type()>> task_package = 
            std::make_shared<std::packaged_task<return_type()>>(std::bind(std::forward<Functor>(f), std::forward<ArgsType>(args)...));

        std::future<return_type> res = task_package->get_future();

        {
            std::unique_lock<std::mutex> lock(m_task_mutex);
            if(m_stop)
                throw std::runtime_error("enqueue on stopped ThreadPool");
            m_tasks.emplace([task_package](){(*task_package)();});
        }

        m_condition.notify_one();
        return res;
    }
    size_t WorkerNums() {return m_workers.size();}
    ~ThreadPool();

private:
    std::vector<std::thread> m_workers;
    std::queue<std::function<void()>> m_tasks;
    bool m_stop;
    std::mutex m_task_mutex;
    std::condition_variable m_condition;
};


#endif