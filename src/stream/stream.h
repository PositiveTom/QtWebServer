#ifndef STREAM_H
#define STREAM_H

#include <iostream>
#include <vector>
#include <memorypool/MemoryPool.h>
#include <stream/stream_line_reader.h>
// #include <server/HttpServer.h>

// using socket_t=int;
typedef int socket_t;

/*类的前向声明, 循环依赖使用*/
namespace http {
    class stream_line_reader;
}

/*输入输出流基类*/
class Stream {
public:
    friend class http::stream_line_reader; /*人为添加, 实际这里破坏了面向对象的依赖倒置原则, 但是你的程序架构目前没法, 后续需要优化*/
    // friend class EpollIO;

    Stream(socket_t sock, std::vector<char, MemoryPool<char>>* buffer, size_t buffer_size) {
        m_sock = sock;
        m_buffer = buffer;
        m_buffer_size = buffer_size;
    }
    virtual ~Stream() = default;
    virtual size_t read_non_block(char* ptr, size_t size) = 0;
    virtual size_t read_all_msg_non_block() = 0;
    std::vector<char, MemoryPool<char>>* GetMemory() {return m_buffer;}

protected:
    socket_t m_sock; /*客户端套接字*/
    std::vector<char, MemoryPool<char>>* m_buffer; /*指向共享内存池的指针*/
    size_t m_buffer_size; /*从共享内存池中取出 m_buffer_size 字节用于当作buffer容器*/
};

/*类A的成员函数中想访问类B的私有属性，必须在类B中声明类A是类B的好朋友类*/
#endif