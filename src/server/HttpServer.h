#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include <memory>
#include <fcntl.h>
#ifdef __linux__
    #include <sys/epoll.h>
#elif

#endif

#include <server/Server.h>
#include <msg/ServerMsg.h>
#include <param/ServerParam.h>
#include <param/TimerParam.h>
#include <stream/SocketStream.h>
#include <stream/stream_line_reader.h>

#include <glog/logging.h>

static constexpr struct EpollParam epoll_param = {
    .epoll_fd_size = 100,
    .epollmaxevents = 100,
    .timeout = -1,
    .is_blocking = false,
};

static constexpr struct ServerCreate server_create_param = {
    .domain = AF_INET,
    .type = 0,
    .protocol = SOCK_STREAM,
    .backlog = 5,
};

static constexpr struct TimerWheelParam timer_wheel_param = {
    .us = 5000,
    .timeout = 5,
};


/*具体类*/
class HttpServer : public Server {
public:
    HttpServer(const struct IpMsg* ipmsg);
    ~HttpServer();

    virtual void StartCreateServer() override;
    virtual int TakeOutTCPConnection() override;  
    virtual void ReadRequest() override;   
    virtual void ProcessRequest() override; 
    virtual void WriteResponse() override; 
};


class EpollIO : public IOMultiplex {
public:
    // friend Server;
    EpollIO(Server* srv) : IOMultiplex(srv) {}
    virtual void MonitorProactorFd() override {} //TODO
    virtual void MonitorReactorFd(const IOMultiplexParam* param) override;
private:
    /*往事件表添加文件描述符*/
    void AddEvent(int epoll_event_fd, int fd, int events, bool is_blocking);
};

class EpollIOFactory : public IOMultiplexFactory {
public:
    virtual std::shared_ptr<IOMultiplex> Create(Server* srv) override;
};


#endif