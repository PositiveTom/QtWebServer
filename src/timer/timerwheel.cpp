#include <timer/timerwheel.h>



std::atomic<TimerWheel*> TimerWheel::m_instance;
std::mutex TimerWheel::m_mutex;

/*-------------------时间轮实现------------------*/

/*采用singlton模式*/
TimerWheel* TimerWheel::getInstance() {
    TimerWheel* tmp = m_instance.load(std::memory_order_relaxed);
    std::atomic_thread_fence(std::memory_order_acquire);
    if(tmp == nullptr) {
        std::lock_guard<std::mutex> lock(m_mutex);
        tmp = m_instance.load(std::memory_order_relaxed);
        if(tmp == nullptr) {
            tmp = new TimerWheel;
            std::atomic_thread_fence(std::memory_order_release);
            m_instance.store(tmp, std::memory_order_relaxed);
        }
    }
    return tmp;
}

void readCurrenttime() {
    m_now[0] = cur_t;                           /*从共享内存中读出当前的时间戳*/
    for(int i=1; i<NUM_LEVELS; i++) {
        m_now[i] = (cur_t >> (i * WIDTH_BITS));
    }
}

/*系统中断调用来触发tick推进*/
void timer_interrupt_handler(int signum) {
    if(signum == SIGALRM) {
        cur_t++;
        readCurrenttime();

        /*时间的推进处理*/
        for(int l=0; l<NUM_LEVELS; l++) {
            // LOG(INFO) << "m_now[l]:" << l << ":" << m_now[l];
            // process_current_slot(m_now[l], max_events, l);
            TimerWheel::m_instance.load(std::memory_order_relaxed)
                ->process_current_slot(m_now[l], std::numeric_limits<Tick>::max(), l);
        }

    }
}

/**
 * @brief 设置一个tick的时间, 中断触发
 * @param us 一个tick等于us微秒
*/
void TimerWheel::startTick(Tick us, Tick now) {
    cur_t = now; /*重置当前tick时间*/

    signal(SIGALRM, timer_interrupt_handler); /*signal.h*/
    struct itimerval t_val; /*<sys/time.h>*/
    t_val.it_interval.tv_sec = 0;
    t_val.it_interval.tv_usec = us;
    t_val.it_value = t_val.it_interval;
    if(setitimer(ITIMER_REAL, &t_val, nullptr) == -1) { /*<sys/time.h>*/
        LOG(FATAL) << "SIGALRM failed";
    }

    for(int i=0; i<NUM_LEVELS; i++) {
        m_now[i] = now >> (WIDTH_BITS * i);
    }
}

/**
 * @brief 设置event从当前时刻开始, 经历delta个Tick周期后开始执行. //TODO 这里寻找槽的方式是真的巧妙啊！！！就是太绕了！！！
 * @param delta delta个Tick周期, 这里的delta的单位是第一个时间轮盘上的单位
*/
void TimerWheel::schedule(TimerEventInterface* event, Tick delta) {
    // assert(delta > 0);
    if(delta <= 0) {
        LOG(ERROR) << "delta:" << delta;
    }
    // m_now[0] = cur_t; /*从共享内存中读出当前的时间戳*/
    // readCurrenttime();
    event->set_scheduled_at(m_now[0] + delta); /*告诉事件,它将在 m_now[0]+delta Tick时刻开始执行*/

    int level = 0;
    while( delta >= NUM_SLOTS ) { /*如果当前转盘延时的时间大于等于一个转盘的时间, 就意味着当前事件需要存放在上一级时间轮盘*/
        /*把 delta 换算成 上一级转盘 需要延时的时间 */
        delta = (delta + (m_now[level] & MASK)) >> WIDTH_BITS;
        ++level;
    }
    /**
     * delta < NUM_SLOTS 时直接是时间轮复用
     * delta >= NUM_SLOTS 时是多层时间轮
    */  
    size_t slot_index = (m_now[level] + delta) & MASK; /*相当于  (m_now[level] + delta) % (2^MASK) */
    auto slot = &m_slots[level][slot_index];
    event->relink(slot);
}

/*在第level层推进delta格*/
bool TimerWheel::advance(Tick delta, size_t max_events, int level) {//TODO
    assert(delta>0);
    if(m_ticks_pending > 0) {

    }

    Tick init_t = m_now[level]; /*记录初始的时间戳*/
    LOG(WARNING) << "begin";
    for(;;) {
        // LOG(INFO) << "m_now[level] - init_t:" << m_now[level] - init_t;
        if((m_now[level] - init_t) == delta) {
            LOG(WARNING) << "end";
            break;
        } else if ((m_now[level] - init_t) > delta) {

        }
        if(!process_current_slot(m_now[level], max_events, level)) {

        }
    }

    return true;
}

bool TimerWheel::myadvance(Tick delta, size_t max_events) {
    Tick init_t = m_now[0]; /*记录初始的时间戳*/
    // LOG(WARNING) << "begin";
    for(;;) {
        // LOG(INFO) << "m_now[level] - init_t:" << m_now[level] - init_t;
        if((m_now[0] - init_t) >= delta) {
            LOG(WARNING) << "end";
            break;
        }
        /*时间的推进处理*/
        for(int l=0; l<NUM_LEVELS; l++) {
            // LOG(INFO) << "m_now[l]:" << l << ":" << m_now[l];
            process_current_slot(m_now[l], max_events, l);
        }
    }
    // LOG(INFO) << "end";
    return true;
}


/*处理level层上对应now时刻的事件*/
bool TimerWheel::process_current_slot(Tick now, size_t max_events, int level) {

    size_t slot_index = now & MASK;
    auto slot = &m_slots[level][slot_index]; /*取出第level个时间轮盘上时间戳为now时对应的槽*/

    while(slot->events()) { /*如果当前槽有事件*/
        // LOG(WARNING) << "event: " << reinterpret_cast<const void*>(slot->events());
        auto event = slot->pop_event(); /*弹出第一个事件*/
        // LOG(WARNING) << "event";
        if(level > 0) {
            Tick cur_time = m_now[0];
            /*如果不是第0层时间轮盘,则需要进行事件的降级操作*/
            if(cur_time > event->scheduled_at()) {
                /*如果事件已经过期, 立即执行*/
                event->execute();
                if(!--max_events) return false;
            } else {
                /*如果事件还没有过期, 进行事件的降级, 通过反复让事件relink, 从而达到让事件接近时间轮复用, 最终变到第一个时间轮盘上进行执行*/
                Tick delta = event->scheduled_at()-cur_time;
                schedule(event, delta);
            }
        } else {
            /*如果当前是第0层时间轮盘*/
            event->execute();
            if(!--max_events) return false; /*直到最大事件数*/
        }
    }
    // LOG(WARNING) << "event";
    return true;
}

/*-------------------槽实现------------------*/

//  无

/*-------------------事件实现------------------*/
void TimerEventInterface::cancel() {
    if(!m_slot) {
        return;
    }
    relink(nullptr);
}

void TimerEventInterface::relink(TimerWheelSlot* new_slot){ /*节点类, 8件事完成relink!!!*/
    // LOG(INFO) << "relink";
    if(new_slot == m_slot) { /*如果相等,则不用插入直接返回*/
        return;
    }
    if(m_slot) { /*如果不相等,则先从原双向链表中移除此事件*/
        auto prev_event = m_prev;
        auto next_event = m_next;
        if(next_event) { /*如果后面有节点*/
            next_event->m_prev = prev_event; /*1.让后一个节点指向前面一个节点*/
        }
        if(prev_event) { /*如果前面有节点*/
            prev_event->m_next = next_event; /*2.让前一个节点指向后面一个节点*/
        } else { /*如果是头节点, 那么就没有前一个节点*/
            LOG(INFO) << reinterpret_cast<void*>(m_slot);
            LOG(INFO) << reinterpret_cast<void*>(m_slot->m_events);
            LOG(INFO) << reinterpret_cast<void*>(next_event);

            m_slot->m_events = next_event;   /*3.移除了头节点, 就要让原来槽的链表头指针指向下一个节点*/
        }
    }

    if(new_slot) { /*插入到新的slot对应的双向链表中, 连接到末尾！！！*/
        /*往新slot的双向链表链接新节点, 为了效率只能连接到头部*/
        auto event = new_slot->m_events; /*获取头节点*/
        m_next = event;                  /*1.让当前节点的下一个节点指向新slot的原头节点*/
        if(event) {                      /*2.如果新slot原来有节点, 让那个节点的前节点指向新插入的节点*/
            event->m_prev = this;
        }
        new_slot->m_events = this;       /*3.更改槽的链表头节点*/
        m_prev = nullptr;                /*4.让头节点的前一个节点为空*/
    } else { /*空节点*/
        m_next = nullptr;
        m_prev = nullptr;
    }
    m_slot = new_slot; /*指向新的槽*/
}