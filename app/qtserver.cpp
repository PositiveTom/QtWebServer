#include <iostream>
#include <glog/logging.h>

int main(int argc, char* argv[]) {
    google::InitGoogleLogging(argv[0]);
    FLAGS_alsologtostderr = true;
    FLAGS_colorlogtostderr = true;

    LOG(WARNING) << "WARNING";

    return 0;
}