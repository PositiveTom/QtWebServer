#ifndef TIMERWHEEL_H
#define TIMERWHEEL_H

/*参考: https://github.com/jsnell/ratas*/

#include <signal.h>
#include <atomic>
#include <mutex>
#include <sys/time.h>
#include <memory>
#include <assert.h>
#include <limits>

#include <glog/logging.h>

typedef uint64_t Tick;

static const int WIDTH_BITS = 8;                                   /*一个轮盘的槽数 2^8 */
static const int NUM_LEVELS = (64 + WIDTH_BITS - 1) / WIDTH_BITS;; /*时间轮盘数*/
static const int MAX_LEVEL = NUM_LEVELS - 1;                       /*最大的时间轮盘id*/
static const int NUM_SLOTS = 1 << WIDTH_BITS;                      /*每个时间轮盘的槽数*/
static const int MASK = (NUM_SLOTS - 1);
static uint64_t cur_t = 0;                                          /*当前时间戳, 以定时器开始计时开始计算, 单位为 tick 周期*/
static Tick m_now[NUM_LEVELS];

class TimerWheelSlot;
class TimerWheel;

/*事件节点--抽象类,可自定义*/
class TimerEventInterface {
public:
    friend TimerWheelSlot;                  /*允许TimerEventInterface类访问类TimerWheelSlot的私有成员*/
    friend TimerWheel;
    TimerEventInterface() : m_slot(nullptr), m_next(nullptr), m_prev(nullptr) {}
    TimerEventInterface(TimerWheelSlot* slot) : m_slot(slot), m_next(nullptr), m_prev(nullptr) {}
    TimerEventInterface(TimerWheelSlot* slot, TimerEventInterface* next, TimerEventInterface* prev) : m_slot(slot), m_next(next), m_prev(prev) {}
    virtual ~TimerEventInterface() {}
    void cancel();                   /*取消事件*/
    Tick scheduled_at() const { return m_scheduled_at; }

protected:
    virtual void execute() = 0;              /*执行事件的回调函数*/
    void relink(TimerWheelSlot* new_slot);  /*把当前事件移动到slot指向的槽里面, 其实就是双向链表的插入*/
    void set_scheduled_at(Tick ts) {m_scheduled_at = ts;}

protected:
    TimerWheelSlot* m_slot;      /*槽*/
    TimerEventInterface* m_next; /*下一个事件节点*/
    TimerEventInterface* m_prev; /*前一个事件节点*/
    Tick m_scheduled_at;         /*当前事件执行的时刻点*/
};

/*时间槽--双向链表*/
class TimerWheelSlot{
public:
    friend TimerEventInterface;
    friend TimerWheel;

    TimerWheelSlot() : m_events(nullptr) {};
    const TimerEventInterface* events() const {return m_events;} /*返回当前双向链表的头节点指针*/
    TimerEventInterface* pop_event() { /*弹出并删除头节点*/
        auto events = m_events;
        
        // LOG(INFO) << reinterpret_cast<void*>(this); // 0x10081ac68
        // LOG(INFO) << reinterpret_cast<void*>(m_events); // 0x0 空地址

        m_events = events->m_next; /*1.让下一个节点变成头节点*/
        if(m_events) {
            /*2.如果有下一个节点, 让下一个节点的前节点指向空*/
            m_events->m_prev = nullptr;
        }
        events->m_slot = nullptr;
        events->m_next = nullptr;
        return events;
    }

private:
    TimerEventInterface* m_events; /*双向链表的头节点, 也是有数据的！！！并不是环形链表*/
};

/*时间轮, 多线程操作同一个共享内存造成段错误！！！！*/
class TimerWheel {
private:
    TimerWheel(){}
    TimerWheel(const TimerWheel* other){}
    ~TimerWheel(){delete m_instance;}

public:
    static TimerWheel* getInstance();                               /*singlton模式创建实例*/
    void startTick(Tick us, Tick now = 0);                          /*初始化相关参数, 以及初始化推进tick用于的SIGALARM信号*/
    void schedule(TimerEventInterface* event, Tick delta);         /*设置事件event在 cur_t + delta 时刻开始运行*/
    inline bool advance(Tick delta, size_t max_events=std::numeric_limits<Tick>::max(), int level=0);  /*在第level层时间轮盘推进delta个tick*/
    // void readCurrenttime(); /*读取当前时间戳并且转换得到各个轮盘上的当前值*/
    inline bool process_current_slot(Tick now, size_t max_events=std::numeric_limits<Tick>::max(), int level=0); /*在now时刻,处理第level层上对应槽的事件*/
    inline bool myadvance(Tick delta, size_t max_events=std::numeric_limits<Tick>::max());

public:
    static std::atomic<TimerWheel*> m_instance;
    static std::mutex m_mutex;

    // Tick m_now[NUM_LEVELS];                                         /*各个时间轮盘经历的Tick周期,全程都是单调递增的*/
    TimerWheelSlot m_slots[NUM_LEVELS][NUM_SLOTS];                     /*所有时间轮盘的槽*/
    Tick m_ticks_pending = 0;                                          /*当前推进还剩余的时间*/
};



#endif