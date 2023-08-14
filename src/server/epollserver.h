#ifndef EPOLLSERVER_H
#define EPOLLSERVER_H

#include <unordered_map>

#include <server/server.h>
#include <sys/epoll.h>
#include <stream/socketstream.h>
#include <stream/streamlinereader.h>

class EpollServer : public Server {
public:
    EpollServer(){}
    virtual void reactorListen(std::string ip, uint16_t port, int backlog, int socket_flags = 0) override {}  //TODO
    virtual void proactorListen(std::string ip, uint16_t port, int backlog, int socket_flags = 0) override;
    virtual bool processAndCloseSocket(socket_t sock) override;
private:
    void epollInit();
    int mEpollFd; //事件表文件描述符号
    std::unordered_map<int, IOCachPtr> mReadyWriteMemory;
};

#endif