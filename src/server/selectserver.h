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
private:
    virtual bool processAndCloseSocket(socket_t sock) override;
    // ssize_t selectRead(socket_t sock);
    bool keepAlive(socket_t sock);
};

#endif