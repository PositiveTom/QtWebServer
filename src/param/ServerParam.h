#ifndef SERVERPARAM_H
#define SERVERPARAM_H

#include <iostream>
#ifdef __linux__
    #include <sys/epoll.h>
    #include <sys/poll.h>
#elif

#endif
#include <sys/select.h>

/*与服务器有关操作的所有参数*/

/*创建服务器需要的相关参数*/
struct ServerCreate {
    /*socket函数参数*/
    int domain;         /*协议簇, bits/socket.h*/
    int type;           /*流服务(TCP) or 数据报(UDP)*/
    int protocol;       /*socket前两个参数就足够了,第3个参数默认为0*/
    
    /*listen函数参数*/
    int backlog;       /*backlog用于存放未处理连接请求的队列的最大长度, 当有大量连接请求到达服务时, 超过该请求队列长度的请求可能会被丢弃和拒绝, 决定了accept中监听队列的大小*/
};

/*建立TCP连接所需要的相关参数*/
struct EstablishTCPConnection {
    /*accept函数参数*/
    struct sockaddr_in* client;     /*客户端ip+端口*/
};

struct IOMultiplexParam {
// public:
//     virtual ~IOMultiplexParam(){}
};

/*epoll所需要的参数*/
struct EpollParam : public IOMultiplexParam {
    /*epoll_create函数*/
    int epoll_fd_size;              /*事件表的尺寸, 并不限制建立了多少个tcp连接*/

    /*epoll_wait函数参数*/
    int epollmaxevents;             /*epoll_wait最大返回的事件数,并不限制建立了多少个tcp连接*/
    // struct epoll_event* epoll_fd;   /*epoll事件表*/
    // std::array<epoll_event, epollmaxevents> epoll_fd;
    int timeout;                    /*超时时间*/

    /*fcntl函数参数, 设置文件描述符非阻塞*/
    /*
        F_SETFL : 设置fd状态标识
        O_NONBLOCK : 设置非阻塞
    */
    bool is_blocking;                /*是否阻塞*/

    /*epoll_ctl函数参数*/
    /*
        EPOLLET : ET模式
        EPOLL_CTL_ADD : 往事件表中注册fd上的事件
    */
    int events; /* EPOLLIN : 数据可读可写 */
};

/*poll所需的参数*/
struct PollParam : public IOMultiplexParam {
    /*poll函数参数*/
    int N;                      /*poll一次性监听的文件描述符个数*/ 
    struct pollfd* poll_fd;     /*存储待监听的文件描述符容器的指针*/
    int nfds;                   /*当前文件描述符容器中有效的文件描述符*/
    int timeout;                /*超时时间*/

    /*fcntl函数参数, 设置文件描述符非阻塞*/
    /*
        F_SETFL : 设置fd状态标识
        O_NONBLOCK : 设置非阻塞
    */
    bool is_blocking;           /*是否阻塞*/
};

/*select所需参数*/
struct SelectParam : public IOMultiplexParam {
    /*select函数参数*/
    /*select函数的第一个参数:要监视的文件描述符中最大的描述符值加 1*/
    fd_set select_fds;          /*select监听的1024个端口, 1024位, 128字节, 索引就是文件描述符*/
    struct timeval tv;          /*select超时时间*/

    bool is_blocking;           /*是否阻塞*/
};


#endif