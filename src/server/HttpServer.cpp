#include <server/HttpServer.h>

HttpServer::HttpServer(const struct IpMsg* ipmsg) : Server(ipmsg) {
    /*监测活动机制*/
}

HttpServer::~HttpServer() {}

/**
 * @brief 创建服务器
*/
void HttpServer::StartCreateServer() {
    m_server_fd = socket(server_create_param.domain, server_create_param.protocol, server_create_param.type);
    assert(m_server_fd >= 0);

    struct sockaddr_in address;
    inet_pton(server_create_param.domain, m_ipmsg->ip.c_str(), &address.sin_addr);
    address.sin_port = htons(m_ipmsg->port);
    address.sin_family = server_create_param.domain;

    int ret = bind(m_server_fd, (struct sockaddr*)(&address), sizeof(address));
    assert(ret != -1);

    ret = listen(m_server_fd, server_create_param.backlog);
    assert(ret != -1);
    LOG(INFO) << "Create Server Succeed!";
}

int HttpServer::TakeOutTCPConnection() {
    // struct sockaddr_in client_addr;
    // socklen_t client_len = sizeof(client_addr);
    // int client_fd = accept(m_server_fd, (struct sockaddr*)nullptr, nullptr);/*这里并不是建立TCP连接，而是从连接队列取出连接*/
    
    // LOG(INFO) << "client_fd: " << client_fd;
    // int client_fd_2 = accept(m_server_fd, (struct sockaddr*)(&client_addr), &client_len);/*这里并不是建立TCP连接，而是从连接队列取出连接*/
    // LOG(INFO) << "client_fd_2: " << client_fd_2;

    // if(client_fd < 0) {
    //     return false;
    // } else {
    //     m_client_fds.emplace_back(client_fd);
    //     return true;
    // }
    return accept(m_server_fd, (struct sockaddr*)nullptr, nullptr);
}

void HttpServer::ReadRequest() {

}

void HttpServer::WriteResponse() {

}

void HttpServer::ProcessRequest() {

}

void EpollIO::MonitorReactorFd(const IOMultiplexParam* param) {
    const EpollParam* epoll_param = static_cast<const EpollParam*>(param);
    /*1.创建事件表*/
    int epoll_event_fd = epoll_create(epoll_param->epoll_fd_size);
    /*2.往事表中添加服务器文件描述符*/
    AddEvent(epoll_event_fd, server->GetSrvfd(), EPOLLIN, epoll_param->is_blocking);
    
    struct epoll_event epoll_events[epoll_param->epollmaxevents];
    /*3.进入监听循环*/
    for(;;) {
        /*4.等待数据传输到来, ret <= epoll_param->epollmaxevents, TL模式下缓冲区只要有数据就一直触发执行*/
        int ret = epoll_wait(epoll_event_fd, epoll_events, epoll_param->epollmaxevents, epoll_param->timeout);
        /*扫描文件描述符*/
        for(int i=0; i<ret; i++) {
            int fd = epoll_events[i].data.fd;
            // LOG(INFO) << "fd:" << fd;
            if( (fd == server->GetSrvfd() ) && (epoll_events[i].events & EPOLLIN) ) {
                /*从监听队列中取出TCP连接*/
                int client_fd = server->TakeOutTCPConnection();
                if(client_fd >= 0) {
                    LOG(INFO) << "Establish or disconnect TCP, client file descriptors:" << client_fd;
                    AddEvent(epoll_event_fd, client_fd, EPOLLIN, epoll_param->is_blocking);
                } else {
                    LOG(WARNING) << "The listen queue is empty!";
                    continue;
                }
            } else if (epoll_events[i].events & EPOLLIN) {
                LOG(INFO) << "Data transfer, client file descriptors:" << fd;
                /*工作线程进行解析Request, 处理Request, 响应Request*/
                ThreadPool* threadpool = server->GetThreadPool();
                threadpool->enqueue([&](){
                    std::vector<char, MemoryPool<char>>* memory = server->RequestMemory(); /*向服务器申请内存池*/
                    SocketStream strm(fd, memory, memory->size());
                    size_t ret = strm.read_all_msg_non_block();                            /*把缓冲队列的数据读取出来*/

                    if(ret != 0) {
                        /*读取到了数据, 且读取到了ret字节*/ //TODO 处理数据
                        std::vector<char, MemoryPool<char>> line_memory(128, '\0');
                        http::stream_line_reader line_reader(strm, ret, line_memory.data(), line_memory.size());
                        line_reader.getline();
                        LOG(INFO) << line_memory.data();
                        line_memory.assign(line_memory.size(), '\0');
                        line_reader.getline();
                        LOG(INFO) << line_memory.data();
                        line_memory.assign(line_memory.size(), '\0');
                        line_reader.getline();
                        LOG(INFO) << line_memory.data();
                        line_memory.assign(line_memory.size(), '\0');
                    }

                    server->ReturnMemory(memory);                                          /*归还内存*/
                });
            }
        }
    }
}

void EpollIO::AddEvent(int epoll_event_fd, int fd, int events, bool is_blocking) {
    struct epoll_event epoll_event;
    epoll_event.data.fd = fd;
    epoll_event.events = events;
    if(!is_blocking) { /*如果非阻塞*/
        epoll_event.events |= EPOLLET; /*设置ET模式*/
        fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);/*设置非阻塞, fcntl.h*/
    }
    epoll_ctl(epoll_event_fd, EPOLL_CTL_ADD, fd, &epoll_event);
}

std::shared_ptr<IOMultiplex> EpollIOFactory::Create(Server* srv) {
    return std::make_shared<EpollIO>(srv);
}