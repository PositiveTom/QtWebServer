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


/*ipv4 tcp 具体服务器类*/
class ConcreteIpv4Server : public AbstractServer {
public:
    ConcreteIpv4Server() {
        LOG(INFO) << "ConcreteIpv4Server()";
    }

    virtual void run(const char* ip, uint16_t port) {
        createServer(ip, port);


        /*
        if(!acceptClient()) {
            perror("run accept");
        }
        LOG(INFO) << "accept success";
        while(true) {
            receiveData();
            sleep(1);
        }
        */

        // sleep(5);

        close(m_server_fd);
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

        ret = listen(m_server_fd, 5);        
        assert(ret != -1);
        LOG(INFO) << "create Server end";
    }

    virtual bool acceptClient() override {
        LOG(INFO) << "accept begin";
        struct sockaddr_in* client = new struct sockaddr_in();
        socklen_t client_length = sizeof(*client);
        // LOG(INFO) << "m_sock:" << m_sock;
        m_server_fd = accept(m_server_fd, (struct sockaddr*)client, &client_length);
        // LOG(INFO) << "m_sock:" << m_sock;
        if(m_server_fd < 0) {
            LOG(INFO) << "accept failure";
            perror("accept");
            return false;
        } else {
            LOG(INFO) << "accept success";
            char remote[INET_ADDRSTRLEN];
            const char* client_ip = inet_ntop(AF_INET, &client->sin_addr, remote, INET_ADDRSTRLEN);
            uint16_t client_port = ntohs(client->sin_port);
            LOG(INFO) << "connected ip: " << client_ip << " port: " << client_port;
            m_clients.push_back((struct sockaddr*)client);
            return true;
        }
    }
    virtual ~ConcreteIpv4Server() {
        LOG(INFO) << "~ConcreteIpv4Server()";
    }
};

// int main(int argc, char* argv[]) {
//     google::InitGoogleLogging(argv[0]);
//     FLAGS_alsologtostderr = true;
//     FLAGS_colorlogtostderr = true;

//     {
//         ConcreteIpv4Server server;
//         server.run("127.0.0.1", 9999);
//     }
//     return 0;
// }