#include <stream/socketstream.h>

SocketStream::SocketStream(socket_t sock):mSock(sock)
{

}

/**
 * @brief 从sock文件描述符端口处读取size个字节
*/
ssize_t SocketStream::read(char* ptr, size_t size) {
    size = std::min(size, std::numeric_limits<size_t>::max());
    

    return 0;
}
