#ifndef HTTPBUFFER_H
#define HTTPBUFFER_H

#include <string>
#include <stream/stream.h>
#include <utils/type.h>

class StreamLineReader {
public:
    StreamLineReader(Stream& strm, IOCachPtr iocaches);
    bool getLine();

private:
    Stream& mStrm;
    IOCachPtr mIOCaches;
    std::string mGrowingBuffers;
};

#endif