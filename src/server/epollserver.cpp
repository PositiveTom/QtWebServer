#include <server/epollserver.h>

void EpollServer::epollInit() {
    mEpollFd = epoll_create(64);
    struct epoll_event event;
    event.data.fd = mSrvsock;
    event.events = EPOLLIN | EPOLLRDHUP |EPOLLET;//当此套接字有数据到来时，et模式只在到来时触发一下
    epoll_ctl(mEpollFd, EPOLL_CTL_ADD, mSrvsock, &event);
}

void EpollServer::proactorListen(std::string ip, uint16_t port, int backlog, int socket_flags) {
    createBindToPort(ip, port, backlog);
    fcntl(mSrvsock, F_SETFL, fcntl(mSrvsock, F_GETFL)|O_NONBLOCK);

    /*创建事件表*/
    epollInit();
    const static int maxevents = 32;
    struct epoll_event events[maxevents];

    bool done = true;
    bool ret = false;
    while((mSrvsock != -1) && done) {
        int fds_ready = epoll_wait(mEpollFd, events, maxevents, -1);

        for(int i=0; i<fds_ready; i++) {
            // int fd_response = events[i].data.fd;
            struct epoll_event& event_res = events[i];

            if(event_res.events & EPOLLRDHUP) {
                /*客户端执行了close套接字*/
                epoll_ctl(mEpollFd, EPOLL_CTL_DEL, event_res.data.fd, nullptr);
                closeSocket(event_res.data.fd);
            } else if(event_res.data.fd == mSrvsock ) {
                /*使用循环，监听此时建立tcp连接的文件描述符*/
                while(true) {
                    /*系统建立了新的tcp连接，取出来加到事件表*/
                    int tcp_fd = accept(mSrvsock, nullptr, nullptr);
                    if(tcp_fd == -1) {
                        if(errno == EMFILE) {
                            /*文件描述符达到上限*/
                            std::this_thread::sleep_for(std::chrono::milliseconds(1));
                            continue;
                        } else if (errno == EAGAIN || errno == EINTR) {
                            /*当前的全连接队列暂时没有数据*/
                            break;
                        }
                        if(mSrvsock != -1) {
                            epoll_ctl(mEpollFd, EPOLL_CTL_DEL, event_res.data.fd, nullptr);
                            closeSocket(mSrvsock);
                        }
                        done = false;
                        break;
                    }
                    fcntl(tcp_fd, F_SETFL, fcntl(tcp_fd, F_GETFL)|O_NONBLOCK);
                    struct epoll_event tcp_event;
                    tcp_event.data.fd = tcp_fd;
                    tcp_event.events = EPOLLIN | EPOLLRDHUP | EPOLLET;
                    epoll_ctl(mEpollFd, EPOLL_CTL_ADD, tcp_fd, &tcp_event);
                }
            } else if (event_res.events & EPOLLIN) {
                /*有数据传输过来，准备读取*/
                {
                    IOCachPtr line_memory = allocateMemory_SingleThread();
                    IOCachPtr io_memory = allocateMemory_SingleThread();

                    SocketStream strm(event_res.data.fd, io_memory);
                    StreamLineReader stream_line_reader(strm, line_memory);
                    ret = !stream_line_reader.getLine();
                    if(ret) {
                        epoll_ctl(mEpollFd, EPOLL_CTL_DEL, event_res.data.fd, nullptr);
                        closeSocket(event_res.data.fd);
                        deallocateMemory_SingleThread(line_memory);
                        deallocateMemory_SingleThread(io_memory);
                        continue;
                    }
                    Request req;
                    ret = !parseRequestLine(stream_line_reader.ptr(), req) || !read_headers(strm, req.headers, line_memory);
                    if(ret) {
                        epoll_ctl(mEpollFd, EPOLL_CTL_DEL, event_res.data.fd, nullptr);
                        closeSocket(event_res.data.fd);
                        deallocateMemory_SingleThread(line_memory);
                        deallocateMemory_SingleThread(io_memory);
                        continue;
                    }
                    Response res;
                    res.version = "HTTP/1.1";
                    writeResponse(strm, req, res, line_memory);
                    if (req.version == "HTTP/1.0" &&
                    req.get_header_value("Connection", 0) != "Keep-Alive") {
                        epoll_ctl(mEpollFd, EPOLL_CTL_DEL, event_res.data.fd, nullptr);
                        closeSocket(event_res.data.fd);
                    }
                    deallocateMemory_SingleThread(line_memory);
                    deallocateMemory_SingleThread(io_memory);
                }
                {
                    // IOCachPtr line_memory = allocateMemory_SingleThread();
                    // IOCachPtr io_memory = allocateMemory_SingleThread();                    

                    // int fd = event_res.data.fd;
                    // mReadyWriteMemory.insert(std::make_pair(fd, line_memory));
                    // mTaskqueue->enqueue([&, line_memory, io_memory, fd](){
                    //     SocketStream strm(fd, io_memory);
                    //     StreamLineReader stream_line_reader(strm, line_memory);
                    //     ret = !stream_line_reader.getLine();
                    //     if(ret) {
                    //         epoll_ctl(mEpollFd, EPOLL_CTL_DEL, fd, nullptr);
                    //         closeSocket(fd);
                    //         deallocateMemory_SingleThread(line_memory);
                    //         deallocateMemory_SingleThread(io_memory);
                    //         return;
                    //     }
                    //     Request req;
                    //     bool ret = !parseRequestLine(stream_line_reader.ptr(), req) || !read_headers(strm, req.headers, line_memory);
                    //     if(ret) {
                    //         epoll_ctl(mEpollFd, EPOLL_CTL_DEL, fd, nullptr);
                    //         closeSocket(fd);
                    //         deallocateMemory_SingleThread(line_memory);
                    //         deallocateMemory_SingleThread(io_memory);
                    //         return;
                    //     }
                    //     Response res;
                    //     res.version = "HTTP/1.1";
                    //     BufferStream bstrm;
                    //     bstrm.write_format(line_memory, "HTTP/1.1 %d OK\r\nContent-Type: text/plain\r\nContent-Length: 5\r\n\r\nhello\r\n", 200);

                    //     /*通知主线程可写数据了*/
                    //     struct epoll_event event;
                    //     event.data.fd = fd;
                    //     event.events = EPOLLOUT | EPOLLET;
                    //     epoll_ctl(mEpollFd, EPOLL_CTL_MOD, fd, &event);
                    //     deallocateMemory_SingleThread(io_memory);
                    //     return;
                    // });
                }
            } else if (event_res.events & EPOLLOUT) {
                /*此文件描述符可以写数据*/
                // if(mReadyWriteMemory.find(event_res.data.fd) != mReadyWriteMemory.end()) {
                //         handle_EAGAIN([&](){
                //             return send(event_res.data.fd, mReadyWriteMemory[event_res.data.fd].get()->data(), strlen(mReadyWriteMemory[event_res.data.fd].get()->data()), 0);
                //         });

                //         deallocateMemory_SingleThread(mReadyWriteMemory[event_res.data.fd]);
                //         mReadyWriteMemory.erase(event_res.data.fd);
                // }
                // epoll_ctl(mEpollFd, EPOLL_CTL_DEL, event_res.data.fd, nullptr);
                // closeSocket(event_res.data.fd);
            }
        }
    }
} 


bool EpollServer::processAndCloseSocket(socket_t sock) {

}