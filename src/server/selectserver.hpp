#include <server/server.hpp>
#include <glog/logging.h>
#include <sys/socket.h>
#include <assert.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <utils.hpp>
#include <sys/select.h>

/*
./../accept_server
telnet 192.168.36.130 9999 模拟客户端连接服务端
*/

/* select 具体服务器类*/
class ConcreteSelectServer : public AbstractServer {
public:
    ConcreteSelectServer() {
        LOG(INFO) << "ConcreteIpv4Server()";
    }

    virtual void run(const char* ip, uint16_t port) {
        createServer(ip, port);
        monitorIoBySelect();
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

        ret = listen(m_server_fd, max_length);  /*5: 等待连接队列的最大长度, 等待连接队列是用于存储传入连接请求但尚未被 accept 函数处理的连接的队列。当有新的连接请求到达时，如果等待连接队列已满，新的连接请求可能会被拒绝或丢弃*/    
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
    
    /*通过select的方式监听IO(文件描述符)*/
    void monitorIoBySelect() {
        /*
            监听的1024个IO, select只监听设置了为1位置的文件描述符是否有数据到来; 
            monitor_fds 位置索引代表tcp连接的文件描述符;
        */
        fd_set monitor_fds;

        struct timeval tv; /*超时时间*/

        int max_fd = m_server_fd; /*值最大的文件描述符*/
        for(;;) {
            tv.tv_sec = 30; /*select 函数会修改tv的剩余时间, 因此必须重新初始化*/
            tv.tv_usec = 0;

            FD_ZERO(&monitor_fds); /*清0*/
            FD_SET(m_server_fd, &monitor_fds); /*服务器文件描述符置1*/

            for(const auto& client_fd : m_client_fds) { /*有连接的客户端文件描述符置1*/
                if(client_fd != 0) {
                    FD_SET(client_fd, &monitor_fds);
                }
            }
            /*最大的文件描述符+1*/
            /*在调用 select 函数后，读集合（readfds）将被修改，只保留就绪状态的文件描述符。可以使用 FD_ISSET 宏来检查特定的文件描述符是否在读集合中，以确定哪些文件描述符已准备好读取数据。*/
            int ret = select(max_fd+1, &monitor_fds, nullptr, nullptr, &tv);/*如果一直没有事件发生, 则等待tv设置的时间后, 再返回*/
            if(ret < 0) {
                perror("select failed");
                LOG(FATAL) << "end";
            } else if (ret == 0) {
                LOG(WARNING) << "timeout";
                continue;
            }

            /* 检查是否有数据到来*/
            for(const auto& client_fd : m_client_fds) {
                if(FD_ISSET(client_fd, &monitor_fds)) {
                    //  TODO: 处理事件
                    FD_CLR(client_fd, &monitor_fds); // 处理完之后, 清空
                    LOG(INFO) << client_fd << " has data";
                }
            }

            /*检查是否有新的tcp连接*/
            if(FD_ISSET(m_server_fd, &monitor_fds)) {
                if(acceptClient()) {
                    FD_CLR(m_server_fd, &monitor_fds);
                }
            }
        }
    }

    void close_all_fds() {
        close(m_server_fd);
        for(const auto& client_fd : m_client_fds) {
            close(client_fd);
        }
    }

    virtual ~ConcreteSelectServer() {
        LOG(INFO) << "~ConcreteSelectServer()";
    }

private:
    std::vector<int> m_client_fds; /*存储建立连接的客户端的IO*/
    const int max_length = 20; /*等待连接队列的最大长度, 当同时有max_length+1个客户端请求连接时, 那么有1个会被拒绝掉*/
};

