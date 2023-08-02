#ifndef SERVER_H
#define SERVER_H

#include <memory>
#include <mutex>
#include <set>
#include <list>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <threadpool/threadpool.h>
#include <memorypool/MemoryPool.h>
#include <timer/timerwheel.h>
#include <param/ServerParam.h>
#include <msg/ServerMsg.h>

#include <glog/logging.h>
#include <QObject>

/*模板方法, 工厂方法, 单例, 观察者(Server是被观察者,ui界面类是观察者)*/

class ThreadPool;
class TimerWheel;
class IOMultiplex;
class IOMultiplexFactory;

class Server {
public:
    Server(const IpMsg* ip_msg);
    virtual ~Server();
    void ProactorMode();    /* //TODO Proactor模式*/
    void ReactorMode(IOMultiplexFactory&& factory, const IOMultiplexParam* param, int mode = 0);     /*Reactor模式*/
    
    void AddObserver(const QObject* window);        /*添加观察者*/
    void RemoveObserver(const QObject* window);     /*移除观察者*/
    
    const std::string& GetIp() const {return m_ipmsg->ip;}
    const uint16_t& GetPort() const {return m_ipmsg->port;}
    const int& GetSrvfd() const {return m_server_fd;}
    // const std::vector<int>& GetClientFd() const {return m_client_fds;}
    ThreadPool* GetThreadPool() {return m_threadpool;}
    TimerWheel* GetTimer() {return m_timer;}
    std::vector<char, MemoryPool<char>>* RequestMemory();/*请求服务器分配内存池*/
    void ReturnMemory(std::vector<char, MemoryPool<char>>* memory); /*归还内存*/
    int AvailableMemory() const; /*返回可用内存池数量*/ 

    virtual void StartCreateServer() = 0;           /*创建ipv4服务器*/
    virtual int TakeOutTCPConnection() = 0;         /*取出TCP连接*/
    // virtual void ReadRequest(int client_fd) = 0;                 /*读请求, 多线程函数*/
    // virtual void ProcessRequest() = 0;              /*处理请求, 多线程函数*/
    // virtual void WriteResponse() = 0;               /*写响应, 多线程函数*/

protected:
    // std::shared_ptr<MemoryPool<char>> m_IOMemory;   /*IO读写缓冲队列内存池*/
    std::vector<std::pair<bool, std::vector<char,MemoryPool<char>>>> m_IOMemory;
    std::vector<std::shared_ptr<std::mutex>> m_IOLocks;

    TimerWheel* m_timer;                            /*时间轮定时器*/
    ThreadPool* m_threadpool;                       /*线程池*/
    const IpMsg* m_ipmsg;                           /*ip地址和端口,用于创建服务器*/
    int m_server_fd;                                /*此服务器的文件描述符*/
    std::shared_ptr<IOMultiplex> m_iomultiplex;     /*IO复用类*/

    // std::vector<int> m_client_fds;              /*存储建立连接的TCP客户端*/
    std::list<const QObject*> m_obs;            /*ui观察者*/
};

/*工厂方法模式创建IO复用对象类, 用于切换poll select epoll*/
class IOMultiplex { /*产品*/
public:
    // friend Server;
    IOMultiplex(Server* srv) : server(srv) {}
    virtual void MonitorProactorFd() = 0; // TODO
    /*监听文件描述符, 并且解析Request, 处理Request, 响应Request*/
    virtual void MonitorReactorFd(const IOMultiplexParam* param) = 0;
    // virtual void ProcessRequest(Stream& stream) = 0;
    virtual ~IOMultiplex(){}
protected:
    Server* server;
};

class IOMultiplexFactory { /*工厂*/
public:
    virtual ~IOMultiplexFactory() {}
    virtual std::shared_ptr<IOMultiplex> Create(Server* srv) = 0;
};


#endif