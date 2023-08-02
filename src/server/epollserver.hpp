#include <server/server.hpp>
#include <glog/logging.h>
#include <sys/socket.h>
#include <assert.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <utils.hpp>
#include <sys/epoll.h>
#include <fcntl.h>
/*
./../accept_server
telnet 192.168.36.130 9999 模拟客户端连接服务端
*/

/* epll 具体服务器类*/
class ConcreteEpollServer : public AbstractServer {
public:
    ConcreteEpollServer() {
        LOG(INFO) << "ConcreteIpv4Server()";
    }

    virtual void run(const char* ip, uint16_t port) {
        createServer(ip, port);
        monitorIoByEpoll();
        close_all_fds();
    }

    void receiveData() {
        LOG(INFO) << "receiving data...";
        char buffer[sizeof(double)];
        int ret = recv(m_server_fd, buffer, sizeof(double), 0);
        
        if(ret <= 0) {
            LOG(INFO) << "there is not enough data";
            return;
        }

        LOG(INFO) << "ret:" << ret;
        LOG(INFO) << "reveive data finish";
        double data;
        recoveryDataFromCharArray(buffer, data);
        LOG(INFO) << "data: " << data;
    }

    void readProcess(int client_fd) {
        const int N = 1500;
        char buffer[N];/*存储临时数据, 后续优化*/
        /**
         * buffer 一般最大为1500字节,网络传输的最大单元
         * len 把结尾字符'\0'留出来,buffer字节数-1
         * flags 一般设置为0
         * @return 正常,读出来的字节大小; 阻塞fd, 会卡在这读, 没数据会阻塞;客户端下线,返回0;
         * 结束条件: 1. buffer已满 2.连接关闭 3.error
        */
        for(;;) {
            memset(buffer, '\0', N);

            int ret = recv(client_fd, buffer, N-1, 0);
            // assert(ret > 0);
            if(ret < 0) {
                if((errno == EAGAIN) || (errno == EWOULDBLOCK) ) {
                    /*数据读取完毕*/
                    LOG(INFO) << "Read finished!";
                    break;
                }
                LOG(ERROR) << "Read Error client:" << client_fd;
                close(client_fd);
                break;
            } else if (ret == 0) {
                /*客户端下线*/
                LOG(WARNING) << "client "<< client_fd <<" off!";
                close(client_fd);
            } else {
                LOG(INFO) << "From client " << client_fd << " get " << ret << " bytes " << "buf:" << buffer;
            }
        }
    }

    /*创建服务器*/
    virtual void createServer(const char* ip, uint16_t port) override {
        LOG(INFO) << "create Server begin";
        m_server_fd = socket(AF_INET, SOCK_STREAM, 0);
        assert(m_server_fd >= 0);
        
        struct sockaddr_in address;
        inet_pton(AF_INET, ip, &address.sin_addr);
        address.sin_port = htons(port);
        address.sin_family = AF_INET;

        int ret = bind(m_server_fd, (struct sockaddr*)(&address), sizeof(address));
        assert(ret != -1);

        ret = listen(m_server_fd, MAX_CLIENTS);  /*5: 等待连接队列的最大长度, 等待连接队列是用于存储传入连接请求但尚未被 accept 函数处理的连接的队列。当有新的连接请求到达时，如果等待连接队列已满，新的连接请求可能会被拒绝或丢弃*/    
        assert(ret != -1);
        LOG(INFO) << "create Server end";
    }

    /*accept与客户端建立连接*/
    virtual bool acceptClient() override {
        LOG(INFO) << "accept begin";
        struct sockaddr_in* client = new struct sockaddr_in();
        socklen_t client_length = sizeof(*client);
        int client_fd = accept(m_server_fd, (struct sockaddr*)client, &client_length);
        if(client_fd < 0) {
            LOG(INFO) << "accept failure";
            perror("accept");
            return false;
        } else {
            LOG(INFO) << "accept success";
            char remote[INET_ADDRSTRLEN];
            const char* client_ip = inet_ntop(AF_INET, &client->sin_addr, remote, INET_ADDRSTRLEN);
            uint16_t client_port = ntohs(client->sin_port);
            LOG(INFO) << "connected ip: " << client_ip << " port: " << client_port << " client_fd:" << client_fd;
            m_clients.push_back((struct sockaddr*)client);
            m_client_fds.push_back(client_fd);
            return true;
        }
    }
    
    /*通过epoll的方式监听IO(文件描述符)*/
    void monitorIoByEpoll() {
        struct epoll_event epoll_events[EPOLLEVENTS];/*存储就绪的epoll事件, 只由epoll_wait修改*/

        int epoll_fd = epoll_create(FDSIZE); /*创建事件表*/
        // LOG(INFO) << EPOLLET;
        // LOG(INFO) << EPOLLIN;
        // LOG(INFO) << (EPOLLET | EPOLLIN);
        add_event(epoll_fd, m_server_fd, EPOLLIN, ENABLE_ET);/*添加服务器端口的文件描述符*/
    
        for(;;) {
            /*等待文件描述符传输数据触发, 返回就绪事件的文件描述符个数*/
            int ret = epoll_wait(epoll_fd, epoll_events, EPOLLEVENTS, -1); /*无限超时,结构体数组epoll_events有EPOLLEVENTS个元素*/

            /*扫描文件描述符*/
            for(int i=0; i<ret; i++) {
                int fd = epoll_events[i].data.fd;
                if((fd == m_server_fd) && (epoll_events[i].events & EPOLLIN) ) {
                    /*处理tcp连接*/
                    acceptClient();
                    add_event(epoll_fd, m_client_fds.back(), EPOLLIN, ENABLE_ET);
                } else if (epoll_events[i].events & EPOLLIN) {
                    /*处理数据*/
                    // TODO
                    readProcess(fd);
                    LOG(INFO) << "process...";
                }
            }
        }
        close(epoll_fd);
    }

    /*往事件表epollfd添加fd*/
    void add_event(int epollfd, int fd, int state, bool enable_et) {
        struct epoll_event epoll_event;
        epoll_event.data.fd = fd;
        epoll_event.events = state;
        if(enable_et) {
            epoll_event.events |= EPOLLET; /*设置ET模式*/
            fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);/*设置非阻塞, fcntl.h*/
        }
        epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &epoll_event);
    }

    void close_all_fds() {
        close(m_server_fd);
        for(const auto& client_fd : m_client_fds) {
            close(client_fd);
        }
    }

    virtual ~ConcreteEpollServer() {
        LOG(INFO) << "~ConcreteSelectServer()";
    }

private:
    std::vector<int> m_client_fds; /*存储建立连接的客户端的IO*/
    const int MAX_CLIENTS = 20; /*等待连接队列的最大长度, 当同时有max_length+1个客户端请求连接时, 那么有1个会被拒绝掉*/
    const int FDSIZE = 1000; /*事件表的尺寸*/
    const int EPOLLEVENTS = 100; /*监听的文件描述符个数*/
    const bool ENABLE_ET = true; /*是否使能ET*/
};

