#pragma once

#include <iostream>
#include <vector>
#include <sys/socket.h>
#include <glog/logging.h>

class AbstractServer {
public:
    AbstractServer() {
        LOG(INFO) << "AbstractServer()";
    }

    virtual void run(const char* ip, uint16_t port) {
        createServer(ip, port);

        if(!acceptClient()) {
            perror("run accept");
        }

        close(m_server_fd);
    }
    virtual void createServer(const char* ip, uint16_t port) = 0;
    virtual bool acceptClient() = 0;
    virtual ~AbstractServer() {
        LOG(INFO) << "~AbstractServer ";
        for(auto& client : m_clients) {
            delete client;
        }
    }
protected:
    int m_server_fd; /*服务器 文件描述符*/
    std::vector<struct sockaddr*> m_clients;
};


