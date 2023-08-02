#ifndef SOCKETSTREAM_H
#define SOCKETSTREAM_H

#include <limits>
#include <cstring>
#include <netinet/in.h>
#include <stream/stream.h>
#include <glog/logging.h>

// #include <memorypool/StackAlloc.h>
// #include <server/HttpServer.h>

int read_socket_non_block(socket_t sock, void* ptr, size_t size);

/*具体类, 一次接收整个报文流*/
class SocketStream : public Stream {
public:
    SocketStream(socket_t sock, std::vector<char, MemoryPool<char>>* buffer, size_t buffer_size) :
        Stream(sock, buffer, buffer_size) {}
    virtual size_t read_non_block(char* ptr, size_t size) override; /*//TODO*/
    virtual size_t read_all_msg_non_block() override; /*非阻塞方式一次性读取缓冲队列所有的数据*/

    // static const size_t m_read_buff_size = 1024 * 4; /*4096字节, 4kb, 没有使用*/
};


#endif