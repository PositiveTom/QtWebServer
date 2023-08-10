#ifndef HTTPBUFFER_H
#define HTTPBUFFER_H

#include <string>
#include <stream/stream.h>
#include <utils/type.h>
#include <utils/tool.h>
#include <httpmsg/request.h>

#ifdef __linux__
#include <assert.h>
#endif
/**
 * @brief 内部有一个buffer，用于保存行数据，行数据块是2048字节
 * 内部还有一个动态buffer
*/
class StreamLineReader {
public:
    StreamLineReader(Stream& strm, IOCachPtr linecaches);
    bool getLine();
    void append(char c);
    const char *ptr() const {
        if (mGrowingBuffers.empty()) {
            return mLineCaches->data();
        } else {
            return mGrowingBuffers.data();
        }
    }
    size_t size() const {
        if(mGrowingBuffers.empty()) {
            return mfixedBufferUsedSize;
        } else {
            return mGrowingBuffers.size();
        }
    }
    bool end_with_crlf() const {
        auto end = ptr() + size();
        return size() >= 2 && end[-2] == '\r' && end[-1] == '\n';/*末尾需要有回车换行符*/
    }
private:
    Stream& mStrm;
    IOCachPtr mLineCaches;          //保存行数据的buffer
    size_t mfixedBufferUsedSize;    //保存行数据的buffer中被使用掉的内存
    size_t mfixedBufferSize;        //保存行数据的内存
    std::string mGrowingBuffers;    //动态buffer
};

#endif