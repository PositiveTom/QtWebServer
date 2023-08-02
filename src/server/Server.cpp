#include <server/Server.h>

Server::~Server() {  }

Server::Server(const IpMsg* ip_msg) {

    m_timer = TimerWheel::getInstance();
    m_threadpool = ThreadPool::getInstance();

    for(int i=0; i<m_threadpool->WorkerNums(); i++) {
        // std::vector<char,MemoryPool<char>> memory;
        m_IOMemory.push_back(std::make_pair(false, std::vector<char,MemoryPool<char>>(4096, '\0')));
        m_IOLocks.emplace_back(std::make_shared<std::mutex>());
    }

    m_ipmsg = ip_msg;
    m_server_fd = -1;
    m_iomultiplex = nullptr;

    // m_client_fds.clear();
    m_obs.clear();

}

void Server::ProactorMode() {

}

void Server::ReactorMode(IOMultiplexFactory&& factory, const IOMultiplexParam* param, int mode) {
    /*1. 创建服务器*/
    StartCreateServer();

    /*2. 创建IO复用类*/
    m_iomultiplex = factory.Create(this);
    
    /*3. 监听文件描述符*/
    m_iomultiplex->MonitorReactorFd(param);
}

void Server::AddObserver(const QObject* window) {
    m_obs.emplace_back(window);
}

void Server::RemoveObserver(const QObject* window) {
    std::list<const QObject*>::iterator it = m_obs.begin();
    if(it != m_obs.end()) {
        m_obs.erase(it);
    }
}

/**
 * @brief 多线程函数, 请求服务器分配一个内存池
*/
std::vector<char, MemoryPool<char>>* Server::RequestMemory() {
    for(int i=0; i<m_IOMemory.size(); i++) {
        std::lock_guard<std::mutex> lock(*(m_IOLocks[i]));
        if(!m_IOMemory[i].first) {
            m_IOMemory[i].first = true;/*加锁！！！！！//TODO*/
            return &(m_IOMemory[i].second);
        }
    }
    return nullptr;
}

/**
 * @brief 归还内存
*/
void Server::ReturnMemory(std::vector<char, MemoryPool<char>>* memory) {
    memory->assign(memory->size(), '\0');
    for(int i=0; i<m_IOMemory.size(); i++) {
        std::lock_guard<std::mutex> lock(*(m_IOLocks[i]));
        if(memory == &(m_IOMemory[i].second)) {
            m_IOMemory[i].first = false;
        }
    }
}

/**
 * @brief 返回可用内存池的块数
*/
int Server::AvailableMemory() const {
    int j = 0;
    LOG(INFO) << "m_IOMemory.size(): " << m_IOMemory.size();
    for(int i=0; i<m_IOMemory.size(); i++) {
        if(!m_IOMemory[i].first) {
            j++;
        }
    }
    return j;
}
