#ifndef BUFFERSTREAM_H
#define BUFFERSTREAM_H

#include <stream/stream.h>
#include <memorypool/MemoryPool.h>

class BufferStream : public Stream {
public:
    using IOCachPtr = std::vector<char, MemoryPool<char>>*;
    BufferStream(int sock, IOCachPtr iocaches);
    virtual ssize_t read(char* ptr, size_t size) override;
private:
    int mSock;           //要读或者写的套接字
    IOCachPtr mIOCaches; //存储IO读写数据的缓存区指针
}; 

#endif