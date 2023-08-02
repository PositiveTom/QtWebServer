#include <ui/mainwindow/main_window.h>
#include <ui/server/serverlogin.h>
#include <ui/server/servermainwindow.h>

#include <glog/logging.h>

int main(int argc, char* argv[]) {
    google::InitGoogleLogging(argv[0]);
    FLAGS_alsologtostderr = true;
    FLAGS_colorlogtostderr = true;

    QApplication app(argc, argv);

    ServerLogin serverlogin;
    int ret = serverlogin.exec();
    LOG(INFO) << "ret:" << ret;

    // MainWindow window;
    // window.show();
    ServerMainWindow server_window;
    server_window.show();


    return app.exec();
}