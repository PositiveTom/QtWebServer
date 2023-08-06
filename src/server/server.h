#ifndef SERVER_H
#define SERVER_H

#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <condition_variable>

#include <sys/socket.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <threadpool/threadpool.h>
#include <memorypool/MemoryPool.h>
#include <utils/type.h>

#include <glog/logging.h>

class Server {
public:
    Server();
    virtual ~Server() = default;
    virtual void reactorListen(std::string ip, uint16_t port, int backlog, int socket_flags = 0) = 0;
protected:
    virtual bool processAndCloseSocket(socket_t sock, IOCachPtr memory_pool) = 0;
    void createBindToPort(std::string ip, uint16_t port, int backlog, int socket_flags = 0);
    int closeSocket(socket_t sock);
    IOCachPtr allocateMemory();
    void deallocateMemory(IOCachPtr memoryblock);

protected:
    socket_t mSrvsock;           //服务器文件描述符
    ThreadPool* mTaskqueue;      //线程池
    time_t mKeepalivemaxcount;   //保持活跃的最大连接数

    QueueMemoryPool mMemorypoll; //线程池
    std::mutex mMemorylock;
    std::condition_variable mCondition;
    size_t mMaxMemoryBlocks;
    size_t mCurrentMemoryBlocks;
};


#endif