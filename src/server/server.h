#ifndef SERVER_H
#define SERVER_H

#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <set>
#include <condition_variable>

#include <sys/socket.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <threadpool/threadpool.h>
#include <memorypool/MemoryPool.h>
#include <timer/timerwheel.h>
#include <utils/type.h>
#include <utils/tool.h>
#include <stream/stream.h>
#include <stream/streamlinereader.h>
#include <stream/bufferstream.h>
#include <httpmsg/request.h>
#include <httpmsg/response.h>

#include <glog/logging.h>

bool read_headers(Stream &strm, Headers &headers, IOCachPtr iocach);

class Server {
public:
    Server();
    virtual ~Server() = default;
    virtual void reactorListen(std::string ip, uint16_t port, int backlog, int socket_flags = 0) = 0;
protected:
    virtual bool processAndCloseSocket(socket_t sock) = 0;
    void createBindToPort(std::string ip, uint16_t port, int backlog, int socket_flags = 0);
    int closeSocket(socket_t sock);
    IOCachPtr allocateMemory();
    void deallocateMemory(IOCachPtr memoryblock);
    bool processRequest(Stream& strm, IOCachPtr line_memory);
private:
    bool parseRequestLine(const char* ptr, Request& req);
    bool writeResponse(Stream& strm, const Request& req, Response& res, IOCachPtr line_memory);
    // bool routing(Request &req, Response &res, Stream &strm);
    // bool writeResponse(Stream& strm, const Request& req, Response& res, IOCachPtr line_memory);

protected:
    socket_t mSrvsock;           //服务器文件描述符
    ThreadPool* mTaskqueue;      //线程池
    time_t mKeepalivemaxcount;   //保持活跃的最大连接数
    uint64_t timeout;            //保活超时时间,毫秒
    TimerWheel* mTimer;

    QueueMemoryPool mMemorypoll; //线程池
    std::mutex mMemorylock;
    std::condition_variable mCondition;
    size_t mMaxMemoryBlocks;
    size_t mCurrentMemoryBlocks;
};


#endif