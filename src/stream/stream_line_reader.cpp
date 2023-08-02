#include <stream/stream_line_reader.h>


namespace http {


stream_line_reader::stream_line_reader(Stream& stream, size_t effetcive_bytes, char *fixed_buffer, size_t fixed_buffer_size) :
    m_stream(stream),
    m_fixed_buffer(fixed_buffer),
    m_fixed_buffer_size(fixed_buffer_size),
    m_stream_offset(0),
    m_effetcive_bytes(effetcive_bytes)
{}

/**
 * @brief 从 stream 流中取出一行, 返回每行的字节数, 不包括回车符号和换行符号
*/
size_t stream_line_reader::getline() {
    // LOG(INFO) << "m_stream_offset:" << m_stream_offset;
    for(size_t index=m_stream_offset, bytes=0; bytes<m_fixed_buffer_size; index++, bytes++) {
        if( ((*m_stream.m_buffer)[index]=='\r') && ((*m_stream.m_buffer)[index+1]=='\n') ) {
            // LOG(INFO) << "end1";
            m_stream_offset += 2;
            m_fixed_buffer[bytes] = '\r';
            m_fixed_buffer[bytes+1] = '\n';
            // LOG(INFO) << "bytes:" << bytes;
            return bytes;
        }
        m_fixed_buffer[bytes] = (*m_stream.m_buffer)[index];
        m_stream_offset++;
    }
    LOG(ERROR) << "m_fixed_buffer_size is not enough";
    // LOG(INFO) << "end2";
    // m_stream_offset+=2;
    return 0;
}

bool stream_line_reader::is_end_with_crlf(int index) {
    return (*m_stream.m_buffer)[index+1] == '\r' && (*m_stream.m_buffer)[index+2] == '\n';
}
    
}
