#ifndef TOOL_H
#define TOOL_H

#include <set>
#include <iostream>
#include <httpmsg/request.h>
#include <sys/socket.h>
#include <utils/type.h>
#include <glog/logging.h>

#ifdef __linux__
  #include <functional>
#endif

/**
 * @brief 阻塞io产生的错误码
 * res = -1， errno = EINTR 这种情况实际上很难遇到
 * res = -1， errno = EAGAIN 当前没有数据可读，程序出错了！！！这种情况在非阻塞io被当作一种正常情况
 * res = 0， 连接已经被关闭
*/
template <typename T>
ssize_t handle_EINTR(T fn) {
    ssize_t res = 0;
    while(true) {
        res = fn();
        if(res < 0 && errno == EINTR) continue;
        break;
    }
    return res;
}

/**
 * @brief 非阻塞io产生的错误码信号
 * res = -1， errno = EINTR 这种情况在select是阻塞的方式时，在检测是否有数据到达时经常遇到
 * res = -1， errno = EAGAIN 当前没有数据可读，需要重复读取, 这种情况似乎很难遇到
 * res = 0， 连接已经被关闭
 * 当没有数据时，res=0， errno = ETIMEDOUT 60，代表操作超时
 * res > 0时，errno = 0才是正常的行为！！！
*/
template <typename T>
ssize_t handle_EAGAIN(T fn) {
    ssize_t res = 0;
    while(true) {
        res = fn();
        // LOG(INFO) << "errno:" << errno;
        if(res < 0 && (errno == EINTR || errno == EAGAIN)) continue;
        // LOG(INFO) << "res:" << res;
        // LOG(INFO) << "errno:" << errno;
        break;
    }
    return res;
}

ssize_t read_socket(socket_t sock, void* ptr, size_t size, int flags);
ssize_t select_read(socket_t sock, time_t sec, time_t usec);

bool compare_case_ignore(const std::string &a, const std::string &b);
bool is_space_or_tab(char c);
void split(const char *b, const char *e, char d,
                  std::function<void(const char *, const char *)> fn);
std::string decode_url(const std::string &s,
                              bool convert_plus_to_space);
void parse_query_text(const std::string &s, Params &params);

template <typename T>
inline bool parse_header(const char *beg, const char *end, T fn) {/*解析http报文中的首部字段,key-value的格式*/
  // Skip trailing spaces and tabs.
  while (beg < end && is_space_or_tab(end[-1])) {/*去掉末尾的空格或者tab转义字符, 例如把 "http://www.baidu.com " "变成"http://www.baidu.com"*/
    end--;
  }

  auto p = beg;
  while (p < end && *p != ':') {/*寻找url中的":"符号"*/
    p++;
  }

  if (p == end) { return false; }

  auto key_end = p;

  if (*p++ != ':') { return false; }/*在运行++之前,*p应该等于":",让p指向下一个字符"*/

  while (p < end && is_space_or_tab(*p)) {/*去掉掉url中:右边的空格或者tab, 例如把" //www.baidu.com"变成 "//www.baidu.com" */
    p++;
  }
  /*url可以写成这种格式:Location: http://www.baidu.com*/
  if (p < end) {
    auto key = std::string(beg, key_end);
    auto val = compare_case_ignore(key, "Location")
                   ? std::string(p, end)
                   : decode_url(std::string(p, end), false);
    fn(std::move(key), std::move(val));
    return true;
  }

  return false;
}


bool has_crlf(const std::string &s);
const char *get_header_value(const Headers &headers,
                                    const std::string &key, size_t id,
                                    const char *def);

#endif