#include <server/selectserver.h>

// bool isCloseSock = false;                  //是否关闭sock
// SockNonActiveEvent sock_event(isCloseSock);//创建事件

SelectServer::SelectServer() : Server() {
}

void SelectServer::reactorListen(std::string ip, uint16_t port, int backlog, int socket_flags) {
    /*创建并把套接字绑定到ip：端口上*/
    createBindToPort(ip, port, backlog);
    LOG(WARNING) << "mSrvsock: " << mSrvsock;

    /*设置非阻塞模式*/
    fcntl(mSrvsock, F_SETFL, fcntl(mSrvsock, F_GETFL) | O_NONBLOCK);
    while(mSrvsock != -1) {
        /*返回就绪文件描述符的数量, 如果accept没有取走就绪文件描述符, 那么这里会一直触发！！！始终返回就绪文件描述符的有效个数，最大是5，因为你的backlog设置是5*/
        // ssize_t val = select_read(mSrvsock);
        // if(val == 0) continue;

        /*从全连接队列中取出一个就绪的文件描述符, 这里取出来的socket一定不是mSorsock*/        
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
        /*设置非阻塞模式*/
        // LOG(WARNING) << "mSrvsock: " << mSrvsock;

        fcntl(sock, F_SETFL, fcntl(sock, F_GETFL) | O_NONBLOCK);
        mTaskqueue->enqueue([this, sock](){
            this->processAndCloseSocket(sock);
        });
    }
}


/**
 * @brief 多线程函数
*/
bool SelectServer::processAndCloseSocket(socket_t sock) {
    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
    // LOG(WARNING) << "thread begin :" << sock << " " << std::this_thread::get_id();
    bool data_has_coming = false;
    time_t count = mKeepalivemaxcount;

    /*申请行内存和buffer内存*/
    IOCachPtr line_memory = nullptr;
    IOCachPtr io_memory = nullptr;
    
    /*设置5s后的定时器事件, 在5s之内有数据到达就取消这个事件，直到下一次运行到这里再设置这个定时器事件，这个事件是设置一个值，告诉系统内已经过了保活时间了，需要取消这个客户端文件描述符*/
    
    while(mSrvsock != -1 && count > 0 && keepAlive(sock)) {
        if(!data_has_coming) {
            line_memory = this->allocateMemory();
            io_memory = this->allocateMemory();
            data_has_coming = true;
        }
        io_memory->assign(io_memory->size(), '\0');
        line_memory->assign(line_memory->size(), '\0');

        // LOG(WARNING) << line_memory->size();
        // LOG(WARNING) << io_memory->size();

        SocketStream strm(sock, io_memory);
        bool ret = this->processRequest(strm, line_memory);
        if(!ret) {
            break;
        }
    }
    if(data_has_coming) {
        this->deallocateMemory(io_memory);
        this->deallocateMemory(line_memory);
    }
    // LOG(WARNING) << line_memory->size();
    // LOG(WARNING) << io_memory->size();
    /*关闭socket*/
    shutdown(sock, SHUT_RDWR);//关闭套接字读写方向
    this->closeSocket(sock);

    LOG(WARNING) << std::this_thread::get_id() << " thread end " << (std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - start)).count() * 1e-3 << "ms";
    return true;
}

// bool SelectServer::keepAlive(socket_t sock) {
//     /*1. 给定时器安排timeout时间之后需要执行的事件*/
//     // bool isCloseSock = false;
//     bool isCloseSock = false;                  //是否关闭sock
//     SockNonActiveEvent sock_event(isCloseSock);//创建事件
//     mTimer->schedule(&sock_event, timeout);    //安排事件在timeouts后准备执行
//     while(true) {
//         ssize_t val = select_read(sock);        //监听是否数据传输过来
//         LOG(INFO) << "VAL:" << val;
//         if(val == -1){                         //代表出错
//             sock_event.cancel();               //取消事件
//             return false;
//         }                      
//         else if(val == 0) {                    //此端口非活跃
//             if(isCloseSock == true) {          //允许的非活跃时间已到
//                 // LOG(INFO) << "isCloseSock:" << isCloseSock;
//                 // LOG(WARNING) << "NON ACTIVE";
//                 return false;                   
//             }
//             // std::this_thread::sleep_for(std::chrono::milliseconds(1));
//             continue;
//             /*cpp-httplib还多了一步延时1ms*/
//         } else {                               //此端口有数据传输过来
//             // LOG(WARNING) << "HELLO";
//             sock_event.cancel();
//             return true;
//         }

//     }
// }


bool SelectServer::keepAlive(socket_t sock) {
    /*1. 给定时器安排timeout时间之后需要执行的事件*/
    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
    while(true) {
        ssize_t val = select_read(sock, 0, 10000);        //监听是否数据传输过来
        if(val == -1){                         //代表出错
            return false;
        }                      
        else if(val == 0) {                    //此端口非活跃
            auto current = std::chrono::steady_clock::now();
            // std::chrono::duration<uint64_t> ms = (current - start).count() * 1000;
            std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(current - start);
            if(ms.count() >= timeout) {
                // std::cout << "ms:" << ms.count() << std::endl;
                return false;
            }
            continue;
        } else {                               //此端口有数据传输过来
            //  正确的情况，这里根本不会等待12ms数据才来，而是1us以内就可以获得数据
            return true;
        }

    }
}