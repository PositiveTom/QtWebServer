#include <stream/socketstream.h>

SocketStream::SocketStream(socket_t sock, IOCachPtr iocache):
mSock(sock), mIOCaches(iocache)
{   
    mIOCaches->assign(mIOCaches->size(), '\0');
    mIOCachSize = mIOCaches->size();
    mReadBufferOff = 0;
    mReadBufferContentSize = 0;
}

/**
 * @brief 从sock文件描述符端口处读取size个字节
 * @param ptr 外部传入的buffer大小
 * 1.期望 size < 缓冲区buffer
 * 直接填满缓冲区buffer
 * ｜---------------｜ 缓冲区buffer
 * ｜--------｜size（期望） ptr
 * 1.1
 * ｜----｜实际读取到的字节数n<=size
 * 从缓冲区buffer里面拿出这n个字节给 ptr
 * 1.2
 * ｜------------｜实际读到的字节数n>size
 * 先从缓冲区buffer取size个字节给ptr
 * 用readBufferContentSize保存n
 * 用readBufferOff保存size
*/
ssize_t SocketStream::read(char* ptr, size_t size) {
    size = std::min(size, std::numeric_limits<size_t>::max());
    
    /**
     * mReadBufferOff: 表示缓冲区内已经读取的字节数
     * mReadBufferContentSize: 表示缓冲区内所有的字节数
    */
    if(mReadBufferOff < mReadBufferContentSize) {// 表明上一次读取没有读取完，buffer缓冲区还有数据
        size_t remainning_size = mReadBufferContentSize - mReadBufferOff;//剩余未读取的字节数
        if(size < remainning_size) {//如果剩余的字节数大于期望读取的字节数
            std::memcpy(ptr, mIOCaches->data()+mReadBufferOff, size);
            mReadBufferOff += size;
            return static_cast<ssize_t>(size);
        } else {//剩余的字节数小于期望读取的字节数
            std::memcpy(ptr, mIOCaches->data()+mReadBufferOff, remainning_size);
            mReadBufferOff += remainning_size;
            return static_cast<ssize_t>(remainning_size);
        }
    }

    if(!is_readable()) return -1;

    mReadBufferOff = 0;
    mReadBufferContentSize = 0;

    if(size < mIOCachSize) {//如果想要读取的字节数小于缓冲区容量大小,则先把数据存储到缓冲区
        ssize_t n = read_socket(mSock, mIOCaches->data(), mIOCachSize, 0);
        if(n <=0 ){//程序出错,或者客户端已经关闭连接
            return n;
        } else if(n <= static_cast<ssize_t>(size)) {//读取的字节数小于期望的字节数
            std::memcpy(ptr, mIOCaches->data(), static_cast<size_t>(n));
            return n;
        } else {
            std::memcpy(ptr, mIOCaches->data(), size);
            mReadBufferOff = size;
            mReadBufferContentSize = static_cast<size_t>(n);
            return static_cast<ssize_t>(size);
        }
    } else {//如果要读取的字节数大于缓冲区容量，则不借助缓冲区
        return read_socket(mSock, ptr, size, 0);
    }
}

bool SocketStream::is_readable() const {
    return select_read(mSock, 5, 0) > 0;
}
