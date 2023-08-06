#include <stream/socketstream.h>

SocketStream::SocketStream(socket_t sock, IOCachPtr iocaches) : 
mSock(sock), mIOCaches(iocaches) 
{

}

ssize_t SocketStream::read(char* ptr, size_t size) {
    return 0;
}
