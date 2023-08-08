#include <stream/bufferstream.h>

ssize_t BufferStream::write(const char* ptr, size_t size) {
    buffer.append(ptr, size);
    return static_cast<ssize_t>(size);
}