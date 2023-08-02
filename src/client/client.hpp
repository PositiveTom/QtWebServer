#pragma once

#include <glog/logging.h>
#include <iostream>

class AbstractClient {
public:
    virtual void run(const char* ip, uint16_t port) {
        createClient();
        if(!connectServer(ip, port)) {
            LOG(INFO) << "failed to connect";
            return;
        } 
        LOG(INFO) << "success to connect";
    }

    virtual void createClient() = 0;
    virtual bool connectServer(const char* ip, uint16_t port) = 0;
    virtual ~AbstractClient() {}

protected:
    int m_sock;
};