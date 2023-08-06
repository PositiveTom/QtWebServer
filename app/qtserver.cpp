#include <iostream>
#include <glog/logging.h>
#include <server/selectserver.h>

int main(int argc, char* argv[]) {
    google::InitGoogleLogging(argv[0]);
    FLAGS_alsologtostderr = true;
    FLAGS_colorlogtostderr = true;

    SelectServer srv;
    std::string ip = "127.0.0.1";
    uint16_t port = 8080;
    srv.reactorListen(ip, port, 5);
    

    return 0;
}