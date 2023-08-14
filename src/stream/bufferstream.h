#ifndef BUFFERSTREAM_H
#define BUFFERSTREAM_H

#include <stream/stream.h>

/**
 * @brief 内部只有一个动态内存
*/
class BufferStream : public Stream {
public:
    virtual bool is_readable() const override {return false;}; //TODO
    virtual ssize_t read(char* ptr, size_t size) override{return 0;}; //TODO
    virtual ssize_t write(const char* ptr, size_t size) override;
    const std::string &get_buffer() const { return buffer; }

private:
    std::string buffer;
};

#endif