#include <server/server.hpp>
#include <glog/logging.h>
#include <sys/socket.h>
#include <assert.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <utils.hpp>
#include <sys/poll.h>

/*
./../accept_server
telnet 192.168.36.130 9999 模拟客户端连接服务端
*/

/* poll 具体服务器类*/
class ConcretePollServer : public AbstractServer {
public:
    ConcretePollServer() {
        LOG(INFO) << "ConcreteIpv4Server()";
    }

    virtual void run(const char* ip, uint16_t port) {
        createServer(ip, port);
        monitorIoByPoll();
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
    
    /*通过poll的方式监听IO(文件描述符)*/
    void monitorIoByPoll() {
        struct pollfd fds[N]; /*poll方式监听的文件描述符数据格式*/
        fds[0].fd = m_server_fd;
        fds[0].events = POLLIN; /*监听是否有tcp连接而设置的*/

        int effective_fd_nums = 1;
        for(int i=1; i<N; i++) {
            fds[i].fd = -1;
        }
        for(;;) {
            int ret = poll(fds, effective_fd_nums, -1); /*无限超时*/
            if(ret == -1) {
                perror("poll error");
                LOG(FATAL) << "exit";
            }

            /*检测是否有新的客户端连接*/
            if(fds[0].revents & POLLIN) {
                acceptClient(); /*接受新的客户端连接*/

                /*把新的连接加入pollfds*/
                for(int i=1; i<N; i++) {
                    if(fds[i].fd < 0) {
                        fds[i].fd = m_client_fds.back();
                        fds[i].events = POLLIN;
                        effective_fd_nums++;
                        break;
                    }
                    if(i == N-1) {
                        LOG(WARNING) << "too many clients";
                        exit(1);
                    }
                }
            }

            /*处理事件*/
            for(int i=1; i<effective_fd_nums; i++) {
                if(fds[i].fd < 0) continue;
                if(fds[i].revents & POLLIN) {
                    /*接受数据, 执行处理流程*/
                    //TODO
                    LOG(INFO) << "process...";
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

    virtual ~ConcretePollServer() {
        LOG(INFO) << "~ConcreteSelectServer()";
    }

private:
    std::vector<int> m_client_fds; /*存储建立连接的客户端的IO*/
    const int max_length = 20; /*等待连接队列的最大长度, 当同时有max_length+1个客户端请求连接时, 那么有1个会被拒绝掉*/
    const int N = 100; /*需要监听的文件描述符数量*/
};

