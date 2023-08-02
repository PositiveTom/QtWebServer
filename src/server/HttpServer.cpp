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

// /**
//  * @brief 从缓冲队列读取数据到buffer对象, 多线程函数
// */
// void HttpServer::ReadRequest(int client_fd) {

// }

// void HttpServer::WriteResponse() {

// }

// /**
//  * @brief 
// */
// void HttpServer::ProcessRequest() {

// }

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
                int client_fd = server->TakeOutTCPConnection();                                 /*从监听队列中取出TCP连接*/
                if(client_fd >= 0) {
                    LOG(INFO) << "Establish or disconnect TCP, client file descriptors:" << client_fd;
                    AddEvent(epoll_event_fd, client_fd, EPOLLIN, epoll_param->is_blocking);     /*把文件描述符添加进事件表*/
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
                    size_t ret = strm.read_all_msg_non_block();                            /*把缓冲队列的数据读取出来保存到内存池*/
                    if(ret != 0) {
                        /*读取到了数据, 且读取到了ret字节*/
                        Request req;                                                       /*存储解析的http报文*/
                        if(ParseHttpMsg(strm, ret, req)) {                                 /*解析http报文*/
                            //  如果成功解析了http报文, 针对报文作出特定的响应
                            
                        }                                      
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
    if(!is_blocking) {                                      /*如果非阻塞*/
        epoll_event.events |= EPOLLET;                      /*设置ET模式*/
        fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);/*设置非阻塞, fcntl.h*/
    }
    epoll_ctl(epoll_event_fd, EPOLL_CTL_ADD, fd, &epoll_event);
}

// void EpollIO::ProcessRequest(Stream& stream) {
// }

/**
 * @brief 解析特定的http报文, 如果是不同的报文, 修改这个函数即可
*/
bool EpollIO::ParseHttpMsg(Stream& strm, int ret, Request& req) {
    std::vector<char, MemoryPool<char>> line_memory(http_work_thread.line_memory, '\0');        /*申请内存用来保存每行的数据*/
    http::stream_line_reader line_reader(strm, ret, line_memory.data(), line_memory.size());    /*创建获取stream逐行数据的类*/
    
    // Request req;
    size_t sum_bytes = strlen(strm.GetMemory()->data()); /*总的字节数*/
    size_t bytes_experienced = 0;                        /*已经遍历过的字节*/
    /*1.获取请求行*/
    {
        bytes_experienced += (line_reader.getline() + 2);
        if(!parse_request_line(line_memory.data(), req)) {
            LOG(WARNING) << "there is invalid requeset header";
            return false;
        }
        // offset += strlen(line_memory.data());
        line_memory.assign(line_memory.size(), '\0');
    }
    /*2.获取首部字段*/
    {   
        // LOG(INFO) << strm.GetMemory()->data();
        size_t line_bytes = 0;
        while( (line_bytes = line_reader.getline()) != 0) {
            // LOG(INFO) << line_memory.data();
            parse_header(line_memory.data(), line_memory.data()+strlen(line_memory.data()),
                    [&](std::string &&key, std::string &&val) {
                        req.m_headers.emplace(std::move(key), std::move(val));
                    });
            line_memory.assign(line_memory.size(), '\0');
            bytes_experienced += (line_bytes+2);
        }
        bytes_experienced += (line_bytes+2);
        // LOG(INFO) << "bytes_experienced:" << bytes_experienced;
        // LOG(INFO) << "sum_bytes:" << sum_bytes;

        // LOG(INFO) << line_reader.getline();
        // LOG(INFO) << line_memory.data();
        // line_memory.assign(line_memory.size(), '\0');
        // LOG(INFO) << line_reader.getline();
        // LOG(INFO) << line_memory.data();
        // line_memory.assign(line_memory.size(), '\0');
        // LOG(INFO) << line_reader.getline();
        // LOG(INFO) << line_memory.data();
        // line_memory.assign(line_memory.size(), '\0');
        // LOG(INFO) << line_reader.getline();
        // LOG(INFO) << line_memory.data();
        // line_memory.assign(line_memory.size(), '\0');
        // LOG(INFO) << line_reader.getline();
        // LOG(INFO) << line_memory.data();
        // line_memory.assign(line_memory.size(), '\0');
        // LOG(INFO) << line_reader.getline();
        // LOG(INFO) << line_memory.data();
        // line_memory.assign(line_memory.size(), '\0');
        // parse_header((*(strm.GetMemory())).data()+strlen(line_memory.data()), (*(strm.GetMemory())).data()+strlen((*(strm.GetMemory())).data()),
        //         [&](std::string &&key, std::string &&val) {
        //         req.m_headers.emplace(std::move(key), std::move(val));
        //         });
        // for(auto&item : req.m_headers) {
        //     LOG(INFO) << item.first << " : " << item.second ;
        // }
    }
    /*3.获取主体*/
    if(bytes_experienced != sum_bytes) {
        /*如果遍历过的字节数小于总的字节数, 还有报文实体*/
        //TODO
    }
    // LOG(INFO) << line_memory.data();
    // LOG(INFO) << strlen(line_memory.data());
    return true;
}

std::shared_ptr<IOMultiplex> EpollIOFactory::Create(Server* srv) {
    return std::make_shared<EpollIO>(srv);
}