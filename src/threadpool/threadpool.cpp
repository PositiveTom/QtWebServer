#include <threadpool/threadpool.h>

/*------------------实现-------------------*/

ThreadPool::ThreadPool() {
    m_stop = false;

    size_t cpu_nums = std::thread::hardware_concurrency();/*获取cpu核心数*/
    LOG(INFO) << "cpu_nums: " << cpu_nums;
    for(size_t i=0; i<cpu_nums ; i++) {
        m_workers.emplace_back(
            [this](){
                for(;;) {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(this->m_task_mutex);
                        this->m_condition.wait(lock, [this](){
                            return !this->m_stop && !this->m_tasks.empty();
                        });

                        if(this->m_stop || this->m_tasks.empty()) {
                            return;
                        }
                        task = this->m_tasks.front();
                        m_tasks.pop();
                    }
                    task();
                }
            }
        );
    }
}

// template<typename Functor, typename... ArgsType>
// auto ThreadPool::enqueue(Functor&& f, ArgsType&&... args) {
//     using return_type = typename std::result_of<Functor(ArgsType...)>::type;

//     std::shared_ptr<std::packaged_task<return_type()>> task_package = 
//         std::make_shared<std::packaged_task<return_type()>>(std::bind(std::forward<Functor>(f), std::forward<ArgsType>(args)...));

//     std::future<return_type> res = task_package->get_future();

//     {
//         std::unique_lock<std::mutex> lock(m_task_mutex);
//         if(m_stop)
//             throw std::runtime_error("enqueue on stopped ThreadPool");
//         m_tasks.emplace([task_package](){(*task_package)();});
//     }

//     m_condition.notify_one();
//     return res;
// }

ThreadPool* ThreadPool::getInstance() {
    static ThreadPool instance;
    return &instance;
}

inline ThreadPool::~ThreadPool() {
    {
        std::unique_lock<std::mutex> lock(m_task_mutex);
        m_stop = true;
    }
    // m_condition.notify_all();
    for(std::thread& thread : m_workers) {
        thread.join();
    }
}