#include <iostream>
#include <glog/logging.h>
#include <server/selectserver.h>
#ifdef __linux__
    #include <server/epollserver.h>
#endif

int main(int argc, char* argv[]) {
    google::InitGoogleLogging(argv[0]);
    FLAGS_alsologtostderr = true;
    FLAGS_colorlogtostderr = true;

    // SelectServer srv;
    // std::string ip = "127.0.0.1";
    // uint16_t port = 8080;
    // srv.reactorListen(ip, port, 5);
    
    #ifdef __linux__
        EpollServer srv;
        std::string ip = "127.0.0.1";
        uint16_t port = 8080;
        srv.proactorListen(ip, port, 5);
    #endif

    return 0;
}