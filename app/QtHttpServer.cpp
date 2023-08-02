#include <memory>
#include <thread>
#include <condition_variable>

#include <glog/logging.h>

#include <msg/ServerMsg.h>
#include <param/TimerParam.h>
#include <server/HttpServer.h>
#include <ui/server/serverlogin.h>
#include <ui/server/servershowwindow.h>


/*监测非活动连接的定时器事件*/
class TimerEventNonActive : public TimerEventInterface {
public:
    TimerEventNonActive(Server* srv) : m_IsEnding(false) {}
    virtual void execte() override {
        // std::unique_lock<std::mutex> lock(this->m_mutex);
        // this->m_con.wait(lock, [])
        // 处理监听事件  //TODO
        
        // m_srv->
        LOG(INFO) << "Process Non active";
        this->m_IsEnding = true;
        this->m_con.notify_one();
    }
    Server* m_srv;
    std::mutex m_mutex;
    std::condition_variable m_con;
    bool m_IsEnding; /*当前事件是否执行完毕*/
};

int main(int argc, char* argv[]) {
    google::InitGoogleLogging(argv[0]);
    FLAGS_alsologtostderr = true;
    FLAGS_colorlogtostderr = true;
    QApplication app(argc, argv);

    std::shared_ptr<HttpServer> http_server;
    std::shared_ptr<TimerEventInterface> timer_event;

    /*1.登陆界面*/
    {
        Ipv4Msg ipv4msg;
        ServerLogin server_login(&ipv4msg);

        if(server_login.exec() == 0) {
            http_server = std::make_shared<HttpServer>(&ipv4msg);
            // LOG(INFO) << "Create HttpServer Succeed!";
        } else {
            LOG(FATAL) << "Create HttpServer Failed!";
        }
    }
    
    /*2.开辟监测非活跃连接线程*/
    // timer_event = std::make_shared<TimerEventNonActive>(http_server.get());
    // std::thread connection_check([&](){
    //     TimerEventNonActive* timer_offical_event = dynamic_cast<TimerEventNonActive*>(timer_event.get());
    //     uint64_t delta = timer_wheel_param.timeout / (timer_wheel_param.us * 1e-6);
    //     http_server->GetTimer()->startTick(timer_wheel_param.us);/*之所以放这里,是因为这个函数触发了中断函数开始*/
    //     http_server->GetTimer()->schedule(timer_event.get(), delta);
    //     for(;;) {
    //         {
    //             std::unique_lock<std::mutex> lock(timer_offical_event->m_mutex);
    //             timer_offical_event->m_con.wait(lock, [&](){
    //                 return timer_offical_event->m_IsEnding;
    //             });
    //             if(timer_offical_event->m_IsEnding) {
    //                 /*执行完毕就再安排一次*/
    //                 http_server->GetTimer()->schedule(timer_event.get(), delta);
    //                 timer_offical_event->m_IsEnding = false;
    //             }
    //         }
    //     }
    // });
    
    /*3.开辟服务器处理线程*/
    /*待添加选择proactor 还是 reactor*/ // TODO
    std::thread main_process([&](){
        /*待添加选择epoll 还是 poll*/ //TODO
        EpollIOFactory factory;
        http_server->ReactorMode(std::move(factory), &epoll_param, 0);
    });

    /*4.ui显示主线程*/
    /*主界面*/
    ServerShowWindow main_window(http_server.get());
    main_window.show();

    return app.exec();
}