#ifndef SELECTSERVER_H
#define SELECTSERVER_H

#include <thread>

#include <server/server.h>
#include <threadpool/threadpool.h>
#include <utils/type.h>
#include <utils/tool.h>
#include <stream/socketstream.h>
#include <chrono>
// #include <stream/streamlinereader.h>

class SockNonActiveEvent : public TimerEventInterface {
public:
    SockNonActiveEvent(bool& isCloseSock):mIsCloseSock(isCloseSock){}
    virtual void execute() override {
        mIsCloseSock = true;
    }
private:
    bool& mIsCloseSock;
};

class SelectServer : public Server {
public:
    SelectServer();
    virtual void reactorListen(std::string ip, uint16_t port, int backlog, int socket_flags = 0) override;
    virtual void proactorListen(std::string ip, uint16_t port, int backlog, int socket_flags = 0) override {} //TODO
    virtual bool processAndCloseSocket(socket_t sock) override;
private:
    bool keepAlive(socket_t sock);
};

//  1. 依赖倒置原则
//  2. 开放封闭原则
//  3. 替换
//  4. 面向接口编程
//  5. 优先使用组合而不是继承
//  6. 单一职责

#endif