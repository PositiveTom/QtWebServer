#ifndef SELECTSERVER_H
#define SELECTSERVER_H

#include <thread>

#include <server/server.h>
#include <threadpool/threadpool.h>
#include <utils/type.h>
#include <utils/tool.h>

class SelectServer : public Server {
public:
    SelectServer();
    virtual void reactorListen(std::string ip, uint16_t port, int backlog, int socket_flags = 0) override;
private:
    virtual bool processAndCloseSocket(socket_t sock) override;
    ssize_t selectRead(socket_t sock);

};

#endif