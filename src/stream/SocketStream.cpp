#include <stream/SocketStream.h>

/*
非阻塞io主要用于单线程同时读取多个io数据时提高并法性能！！
非阻塞模式需要人来告诉计算机是否需要停止读取数据了！！！
非阻塞模式不需要设置超时时间
非阻塞模式必须要放在主线程才行, 因为只有主线程才能一直运行, 而工作线程不能一直运行, 因此工作线程读必须用阻塞模式
recv本身不知道容量buf的大小, 假设要读取10个字节
recv < 0 表示当前没有数据可读
recv = 0 客户端关闭
recv > 0 则可能返回0～10之间的任何一个数，那么此时需要人告诉计算机你下一次的数据应该存放的位置

目前这个读取函数, 只适用于, buffer(4KB) 内存大于等于缓冲区队列数据的情况
*/
int read_socket_non_block(socket_t sock, char* ptr, size_t size) {
    int ret = 0;
    int cur_bytes_read = 0; /*读取的总字节数*/
    for(;;) {
        if(size - ret == 0) break;
        // memset(ptr, '\0', size);
        /*
            recv函数读取了 (size-ret) 个字节才会返回有效值, 又或者把 缓冲队列的数据 读完
            只要有字节读取, ret值肯定是大于0的！！！！
        */
        ret = recv(sock, ptr+ret, size-ret, 0);/*读取size个字节存储到ptr指向的内存空间上*/
        // LOG(INFO) << "ret:" << ret;
        if(ret < 0) {
            if((errno == EAGAIN) || (errno == EWOULDBLOCK) || (errno == EINTR)) {
                /*非阻塞模式,表示此时没有数据可读, 或者已经读够了*/
                // LOG(INFO) << "non block";
                break;
            }
            LOG(FATAL) << "read error!";
        } else if (ret == 0) {
            /*客户端下线, 连接关闭*/
            LOG(WARNING) << "client "<< sock <<" off!";
            close(sock);
            return ret;
        } else {
            /*buffer被塞满了, 或者缓冲区没有数据了*/
            cur_bytes_read = ret;
            // LOG(INFO) << "From client " << sock << " get " << ret << " bytes ";
            // return ret;
        }
    }
    return cur_bytes_read;
}

/**
 * @brief 一次性从文件描述符sock中读取size个字节存储到ptr指向的内存空间, size的大小就是ptr的大小
*/
size_t SocketStream::read_non_block(char* ptr, size_t size) {
    // size = std::min(size, std::numeric_limits<size_t>::min()); /*限制非法数字*/

    // if(size >= m_read_buff_size) {
    //     /*如果期望读取的字节数大于缓冲区容量, 则不使用缓冲区的内存*/
    //     return read_socket_non_block(m_sock, ptr, size);        
    // } else {
    //     /*如果期望读取的字节数小于缓冲区容量, 则使用缓冲区的内存*/
    //     int n = read_socket_non_block(m_sock, ptr, size);
    //     if(n < 0) {
    //         /*此时缓冲区数据读取完毕*/
    //         return n;
    //     } else {

    //     }
    // }
    return 1;
}

/**
 * @brief 非阻塞方式一次性读取缓冲队列所有的数据, 返回读取的字节数
*/
size_t SocketStream::read_all_msg_non_block() {
    return read_socket_non_block(m_sock, m_buffer->data(), m_buffer_size);
}