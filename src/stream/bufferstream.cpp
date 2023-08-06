#include <stream/bufferstream.h>

BufferStream::BufferStream(socket_t sock, IOCachPtr iocaches) : 
mSock(sock), mIOCaches(iocaches) 
{

}

ssize_t BufferStream::read(char* ptr, size_t size) {
    return 0;
}
