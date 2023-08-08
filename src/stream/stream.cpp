#include <stream/stream.h>

// size_t Stream::write(const char* ptr) {
//     return write(ptr, strlen(ptr));
// }

// size_t Stream::write(std::string data) {
//     return write(data.data(), data.size());
// }

// template <typename... Args>
// ssize_t Stream::write_format(IOCachPtr iocach, const char* fmt, const Args &...args) {
//     iocach->assign(iocach->size(), '\0');

//     /*返回格式化的字符串长度，从格式化字符串fmt中取出iocach->size()-2个字节放到iocach->data()指向的内存空间*/
//     ssize_t su = snprintf(iocach->data(), iocach->size()-1, fmt, args...);
//     if(su <= 0) return su;

//     size_t n = static_cast<size_t>(su);
//     /*如果格式化的字符串长度大于缓冲区的长度, 使用一个动态扩容内存*/
//     if(n >= iocach->size() - 1) {
//         std::vector<char> growable_buf(iocach->size());

//         while (n >= growable_buf.size() - 1) {
//             growable_buf.resize(growable_buf.size() * 2);
//             n = static_cast<size_t>(snprintf(growable_buf.data(), growable_buf.size()-1, fmt, args...));
//         }
//         return write(growable_buf.data(), growable_buf.size());
//     } else {
//         write(iocach->data(), n);
//     }

// }
