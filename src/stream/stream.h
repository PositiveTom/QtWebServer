#ifndef STREAM_H
#define STREAM_H

#include <iostream>
#include <string>

class Stream {
public:
    virtual ~Stream() = default;

    // virtual bool is_writable() const = 0;
    // virtual bool is_readable() const = 0;

    virtual ssize_t read(char* ptr, size_t size) = 0;
    // virtual size_t write(const char* ptr, size_t size) = 0;

    // virtual int socket() const = 0;
    // virtual size_t write(const char* ptr);
    // virtual size_t write(std::string data);
};

#endif