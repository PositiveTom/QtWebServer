#ifndef EPOLLSERVER_H
#define EPOLLSERVER_H

#include <server/server.h>

class EpollServer : public Server {
public:
    EpollServer() {}
    virtual void reactorListen(std::string ip, uint16_t port, int backlog, int socket_flags = 0) override {}
    virtual void proactorListen(std::string ip, uint16_t port, int backlog, int socket_flags = 0) override; //TODO
private:
    virtual bool processAndCloseSocket(socket_t sock) override;
};

#endif