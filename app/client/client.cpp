#include <client/ipv4client.hpp>


int main(int argc, char* argv[]) {
    google::InitGoogleLogging(argv[0]);
    FLAGS_alsologtostderr = true;
    FLAGS_colorlogtostderr = true;

    {
        Client1 client;
        client.run("127.0.0.1", 9999);
    }

    return 0;
}