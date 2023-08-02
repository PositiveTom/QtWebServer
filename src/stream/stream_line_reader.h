#ifndef STREAM_LINE_READER_H
#define STREAM_LINE_READER_H

#include <stream/stream.h>
#include <glog/logging.h>
class Stream;

namespace http {

/**
 * @brief http服务器专属类, 读取http报文中的一行数据
 */
class stream_line_reader {
public:
    /**
     * @brief
     * @param stream 数据流
     * @param fixed_buffer 存储一行数据的容器指针
     * @param fixed_buffer_size 这个容器的大小
    */
    stream_line_reader(Stream& stream, size_t effetcive_bytes, char *fixed_buffer, size_t fixed_buffer_size);
    size_t getline(); /*从stream中获取一行数据*/

private:
    bool is_end_with_crlf(int index); /*是否是最后一个字符,包括换行符号*/

    Stream& m_stream;
    char* m_fixed_buffer;
    size_t m_fixed_buffer_size;
    size_t m_effetcive_bytes; /*stream中的有效字节数*/

    size_t m_stream_offset; /*stream中数据行首地址偏移量*/
};

}

#endif
