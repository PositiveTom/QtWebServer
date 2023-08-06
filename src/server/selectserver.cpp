#include <server/selectserver.h>

SelectServer::SelectServer() : Server() {
}

void SelectServer::reactorListen(std::string ip, uint16_t port, int backlog, int socket_flags) {
    /*创建并把套接字绑定到ip：端口上*/
    createBindToPort(ip, port, backlog);
    LOG(WARNING) << "mSrvsock: " << mSrvsock;

    /*设置非阻塞模式*/
    fcntl(mSrvsock, F_SETFL, fcntl(mSrvsock, F_GETFL) | O_NONBLOCK);

    while(mSrvsock != -1) {
        /*返回就绪文件描述符的数量, 如果accept没有取走就绪文件描述符,那么这里会一直触发！！！始终返回就绪文件描述符的有效个数，最大是5，因为你的backlog设置是5*/
        ssize_t val = selectRead(mSrvsock);
        if(val == 0) continue;

        /*从全连接队列中取出一个就绪的文件描述符*/        
        socket_t sock = accept(mSrvsock, nullptr, nullptr); 
        if(sock == -1) {
            if(errno == EMFILE) {
                /*代表文件描述符已经达到上限, 进入休眠期, 给操作系统一些时间来释放资源*/
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                continue;
            } else if(errno == EINTR || errno == EAGAIN) {
                /**
                 * EINTR: 当前系统调用没有成功完成
                 * EAGAIN: 当前系统调用由于没有足够资源完成操作
                */
               continue;
            }
            if(mSrvsock != -1) {
                closeSocket(mSrvsock);
            }
            break;
        }
        mTaskqueue->enqueue([this, sock](){
            this->processAndCloseSocket(sock, this->allocateMemory());
        });
    }
}

ssize_t SelectServer::selectRead(socket_t sock) {
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(mSrvsock, &fds);

    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 0;

    return handle_EINTR([&](){
        return select(static_cast<int>(sock+1), &fds, nullptr, nullptr, &tv);
    });
}

/**
 * @brief 多线程函数
*/
bool SelectServer::processAndCloseSocket(socket_t sock, IOCachPtr memory_pool) {
    time_t count = mKeepalivemaxcount;
    while(mSrvsock != -1 && count > 0) {
        SocketStream strm(mSrvsock);
        StreamLineReader stream_line_reader(strm, memory_pool);
        


    }


    this->deallocateMemory(memory_pool);
    return true;
}
