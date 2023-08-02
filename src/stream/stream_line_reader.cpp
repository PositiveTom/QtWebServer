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
 * @brief 从 stream 流中取出一行
*/
size_t stream_line_reader::getline() {
    for(size_t index=m_stream_offset, bytes=0; index<m_fixed_buffer_size; index++, bytes++) {
        if( ((*m_stream.m_buffer)[index]=='\r') && ((*m_stream.m_buffer)[index+1]=='\n') ) {
            m_stream_offset += 2;
            return bytes;
        }
        m_fixed_buffer[bytes] = (*m_stream.m_buffer)[index];
        m_stream_offset++;
    }
    return 0;
}

bool stream_line_reader::is_end_with_crlf(int index) {
    return (*m_stream.m_buffer)[index+1] == '\r' && (*m_stream.m_buffer)[index+2] == '\n';
}
    
}
