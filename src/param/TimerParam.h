#ifndef TIMERPARAM_H
#define TIMERPARAM_H

#include <iostream>

struct TimerWheelParam {
    uint64_t us;    /*us微秒触发一次定时器中断, 也是一个Tick周期*/
    double timeout; /*每timeout时间检查一下TCP连接是否活动*/
};

#endif