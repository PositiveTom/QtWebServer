#include <timer/timerwheel.h>
#include <unistd.h>

std::mutex flag_mutex;
bool flag = true;

class TimerEvent1 : public TimerEventInterface {
public:
    TimerEvent1(){}
    virtual void execte() override {
        std::unique_lock<std::mutex> lock(flag_mutex);
        flag = true;
        LOG(INFO) << "TimerEvent1 Process...";
    }
};

/*

I20230724 10:41:44.170496 57639 timer.cpp:21] begin
I20230724 10:41:46.670490 57639 timer.cpp:8] TimerEvent1 Process...
W20230724 10:41:52.170493 57639 timerwheel.h:216] end

*/

int main(int argc, char* argv[]) {
    google::InitGoogleLogging(argv[0]);
    FLAGS_alsologtostderr = true;
    FLAGS_colorlogtostderr = true;

    TimerWheel* timer_wheel = TimerWheel::getInstance();
    // timer_wheel->advance(2 / (500000 * 1e-6));
    TimerEvent1* event1 = new TimerEvent1;
    timer_wheel->startTick(5000); /*设置一个tick的周期时长, 微秒*/
    LOG(INFO) << "begin";
    // timer_wheel->schedule(event1, 1.0 / (5000 * 1e-6)); /*设置一个任务将在2.5s后执行*/
    // timer_wheel->myadvance(8 / (5000 * 1e-6)); /*设置整个定时器推进8s*/
    // event1->execte();

    while(true) {
        if(flag == true) {
            Tick delta = 1.0 / (5000 * 1e-6);
            timer_wheel->schedule(event1, delta);
            std::unique_lock<std::mutex> lock(flag_mutex);
            flag = false;
        }
    }

    return 0;
}