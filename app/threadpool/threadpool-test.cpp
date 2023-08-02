#include <threadpool/threadpool.h>


int main(int argc, char** argv) {
    
    google::InitGoogleLogging(argv[0]);
    FLAGS_colorlogtostderr = true;
    FLAGS_alsologtostderr = true;

    ThreadPool* threadpool = ThreadPool::getInstance();
    // LOG(INFO) << std::thread::hardware_concurrency();
    std::vector<std::future<void>> results;
    for(int i=0; i<4; i++)
        results.emplace_back(threadpool->enqueue([](){
            LOG(INFO) << "hello threadpool";
        }));

    for(int i=0; i<4; i++) {
        results[i].get();
    }
    results.clear();
    for(int i=0; i<4; i++)
        results.emplace_back(threadpool->enqueue([](){
            LOG(INFO) << "hello threadpool";
        }));
    for(int i=0; i<4; i++) {
        results[i].get();
    }
    return 0;
}