#include <glog/logging.h>
#include <iostream>
#include <client/client.hpp>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <utils.hpp>

/*concrete client*/
class Client1 : public AbstractClient {
public:
    virtual void run(const char* ip, uint16_t port) {
        createClient();
        if(!connectServer(ip, port)) {
            LOG(INFO) << "failed to connect";
            return;
        }
        LOG(INFO) << "success to connect";
        while(true) {
            LOG(INFO) << "send";
            sendData(11.238);
            sleep(1);
        }
        close(m_sock);
    }

    template <typename T>
    void sendData(const T& data) { // const & 也会延长右值的生命周期
        assert(m_sock >= 0);
        char data_char[sizeof(data)];
        saveDataToCharArray(data, data_char);
        LOG(INFO) << "data to be sent: " << data;
        int ret = send(m_sock, data_char, sizeof(data), 0);
        // perror("send");
        LOG(INFO) << "sent: " << ret << " bytes";
        assert(ret > 0);
    }

    virtual void createClient() override {
        LOG(INFO) << "create Client1 begin";
        m_sock = socket(AF_INET, SOCK_STREAM, 0);
        // LOG(INFO) << "m_sock:" << m_sock;
        assert(m_sock >= 0);
        LOG(INFO) << "create Client1 end";
    }

    virtual bool connectServer(const char* ip, uint16_t port) override {
        struct sockaddr_in address;
        bzero(&address, sizeof(address));
        inet_pton(AF_INET, ip, &address.sin_addr);
        address.sin_port = htons(port);
        address.sin_family = AF_INET;
        // LOG(INFO) << "m_sock:" << m_sock;
        int ret = connect(m_sock, (struct sockaddr*)&address, sizeof(address));
        // LOG(INFO) << "m_sock:" << m_sock;
        if(ret < 0) {
            perror("connect");
            return false;
        } else {
            return true;
        }
    }
};


// int main(int argc, char* argv[]) {
//     google::InitGoogleLogging(argv[0]);
//     FLAGS_alsologtostderr = true;
//     FLAGS_colorlogtostderr = true;

//     {
//         Client1 client;
//         client.run("127.0.0.1", 9999);
//     }

//     return 0;
// }