#ifndef SERVER_H
#define SERVER_H

#include <threadpool/threadpool.h>

class Server {
public:


protected:
    ThreadPool* taskqueue;      //线程池
};


#endif