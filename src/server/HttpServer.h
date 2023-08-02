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
#include <param/HttpParam.h>
#include <stream/SocketStream.h>
#include <stream/stream_line_reader.h>
#include <http/Request.h>
#include <http/Response.h>

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

static constexpr struct HttpWorkThreadParam http_work_thread = {
    .line_memory = 128,
};

/*具体类*/
class HttpServer : public Server {
public:
    HttpServer(const struct IpMsg* ipmsg);
    ~HttpServer();

    virtual void StartCreateServer() override;
    virtual int TakeOutTCPConnection() override;  
    // virtual void ReadRequest(int client_fd) override;   
    // virtual void ProcessRequest() override; 
    // virtual void WriteResponse() override; 
};

class EpollIO : public IOMultiplex {
public:
    // friend Server;
    EpollIO(Server* srv) : IOMultiplex(srv) {}
    virtual void MonitorProactorFd() override {} //TODO
    virtual void MonitorReactorFd(const IOMultiplexParam* param) override;
    // virtual void ProcessRequest(Stream& stream) override;
private:
    /*往事件表添加文件描述符*/
    void AddEvent(int epoll_event_fd, int fd, int events, bool is_blocking);
    bool ParseHttpMsg(Stream& stream, int ret, Request& req); /*解析http报文, EpollIO类特定的函数*/

};

class EpollIOFactory : public IOMultiplexFactory {
public:
    virtual std::shared_ptr<IOMultiplex> Create(Server* srv) override;
};


#endif