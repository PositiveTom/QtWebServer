// #include <server/selectserver.hpp>
// #include <server/pollserver.hpp>
#include <server/epollserver.hpp>

int main(int argc, char* argv[]) {
    google::InitGoogleLogging(argv[0]);
    FLAGS_alsologtostderr = true;
    FLAGS_colorlogtostderr = true;

    {
        // ConcreteSelectServer server;
        // server.run("127.0.0.1", 9999);
        // ConcretePollServer server;
        // server.run("127.0.0.1", 9999);
        ConcreteEpollServer server;
        server.run("127.0.0.1", 9999);
    }
    return 0;
}