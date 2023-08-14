#ifndef SOCKETSTREAM_H
#define SOCKETSTREAM_H

#include <limits>
#include <stream/stream.h>
#include <utils/type.h>
#include <utils/tool.h>

/**
 * @brief 内部有一个buffer, buffer的大小是4096字节, 外部传入一个buffer指针 (这里的buffer指针的数据 需要组合成行传给 StreamLineReader，一个字符一个字符地保存)
 * 更改成2048字节
*/
class SocketStream : public Stream {
public:
    SocketStream(socket_t sock, IOCachPtr iocache);
    virtual ssize_t read(char* ptr, size_t size) override;
    virtual ssize_t write(const char* ptr, size_t size) override {return 0;}; //TODO
    virtual int socket() const override {return mSock;}
    virtual bool is_readable() const override;
private:
    socket_t mSock;         //要读或者写的套接字
    IOCachPtr mIOCaches;    //保存从文件描述符读取的数据
    size_t mIOCachSize;     //读buffer缓冲区大小  
    size_t mReadBufferOff;
    size_t mReadBufferContentSize;
};

#endif