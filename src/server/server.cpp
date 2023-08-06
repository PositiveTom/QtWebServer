#include <server/server.h>

Server::Server() {
    mTaskqueue = ThreadPool::getInstance();
    mKeepalivemaxcount = 5;
    mMaxMemoryBlocks = 20;
    for(size_t i=0; i<mTaskqueue->WorkerNums(); i++) {
        mMemorypoll.push(std::make_shared<IOCach>(4096, '\0'));
    }
    mCurrentMemoryBlocks = mMemorypoll.size();
}

void Server::createBindToPort(std::string ip, uint16_t port, int backlog, int socket_flags) {
    while(true) {
        /*创建socket*/
        mSrvsock = socket(AF_INET, SOCK_STREAM, socket_flags);
        if(mSrvsock == -1) continue;
        /**
         * fcntl: 对文件描述符号进行控制操作的系统调用
         * F_SETFD: 设置文件描述符标志的操作
        */
        if(fcntl(mSrvsock, F_SETFD, FD_CLOEXEC) == -1) { //fork后，执行子进程时关闭父进程里面的该文件描述符号
            closeSocket(mSrvsock);
            continue;
        }        
        int yes = 1;
        setsockopt(mSrvsock, SOL_SOCKET, SO_REUSEPORT, reinterpret_cast<const void*>(&yes), sizeof(yes)); //允许多个套接字绑定到同一个端口

        struct sockaddr_in address; //地址转换
        inet_pton(AF_INET, ip.c_str(), &address.sin_addr);
        address.sin_family = AF_INET;
        address.sin_port = htons(port);

        if((bind(mSrvsock, (struct sockaddr*)(&address), sizeof(address)) == -1) || 
           (listen(mSrvsock, backlog) == -1)) {
            closeSocket(mSrvsock);
            continue;
        }
        break;
    }
    LOG(INFO) << "Create, bind and listen succed!";
}

int Server::closeSocket(socket_t sock) {
    return close(sock);
}

/**
 * @brief 从服务器的内存池块中分配一块可用的内存池给子线程
*/
IOCachPtr Server::allocateMemory() {
    std::unique_lock<std::mutex> lock(mMemorylock);
    while(mMemorypoll.empty() && mCurrentMemoryBlocks >= mMaxMemoryBlocks) {
        mCondition.wait(lock);
    }
    IOCachPtr memoryBlock = nullptr;
    if(!mMemorypoll.empty()) {
        memoryBlock = mMemorypoll.front();
        mMemorypoll.pop();
    } else {
        memoryBlock = std::make_shared<IOCach>(4096, '\0');
        ++mCurrentMemoryBlocks;
    }
    return memoryBlock;
}

/**
 * @brief 归还内存给服务器
*/
void Server::deallocateMemory(IOCachPtr memoryblock) {
    std::lock_guard<std::mutex> lock(mMemorylock);
    memoryblock->assign(memoryblock->size(), '\0');
    mMemorypoll.push(memoryblock);
    mCondition.notify_one();
}


