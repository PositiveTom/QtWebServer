#include <utils/tool.h>

//  1.首先系统不会无缘无故的调用recv函数，调用了recv函数肯定是事先知道了有数据才进行读，并且有数据的结束符号告诉系统不需要读数据了
//  2.EAGAIN就是EWOULDBLOCK的宏定义，因此二者相等
//  3.

/**
 * @brief
 * 
 * 阻塞模式下调用recv函数：如果没有数据，如果没有设置超时时间，程序会被一直阻塞
 * 如果设置为一个非负数，则在等待一段时间后仍然没有数据，则返回-1，并设置errno为 EAGAIN/EWOULDBLOCK，表示当前没有数据可用
 * 非阻塞模式下调用recv函数：没有数据，立即返回-1，并将errno设置为 EAGAIN/EWOULDBLOCK，表示当前没有数据可用
 * 
*/
ssize_t read_socket(socket_t sock, void* ptr, size_t size, int flags) {
    return handle_EAGAIN([&](){
        return recv(sock, ptr, size, flags);
    });
}


//  一旦建立连接，就开始统计是否有数据到达，没有数据到达意味着非活动连接，5s非活动连接就准备关闭

/**
 * @brief 监听 sock 文件描述符是否有 有操作就绪 
 * 
 * 输入服务器的套接字，代表监听是否有tcp连接
 * 输入客户端的套接字，代表监听是否有数据到来
 * 
 * -1 代表没有文件描述符
*/
ssize_t select_read(socket_t sock) {
    if(sock >= FD_SETSIZE) return 1;

    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(sock, &fds);

    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 10000;

    /*select 不受io端口阻塞的影响！！！！只受自己的超时时间影响*/
    /*select 函数，如果端口的数据一直没有被读取完，那么会一直触发！！！*/
    return handle_EAGAIN([&](){
        return select(static_cast<int>(sock+1), &fds, nullptr, nullptr, &tv);
    });
}

bool is_space_or_tab(char c) { return c == ' ' || c == '\t'; }

inline std::pair<size_t, size_t> trim(const char *b, const char *e, size_t left,
                                      size_t right) {
  while (b + left < e && is_space_or_tab(b[left])) {
    left++;
  }
  while (right > 0 && is_space_or_tab(b[right - 1])) {
    right--;
  }
  return std::make_pair(left, right);
}

void split(const char *b, const char *e, char d,
                  std::function<void(const char *, const char *)> fn) {
  size_t i = 0;
  size_t beg = 0;

  while (e ? (b + i < e) : (b[i] != '\0')) {
    if (b[i] == d) {
      auto r = trim(b, e, beg, i);
      if (r.first < r.second) { fn(&b[r.first], &b[r.second]); }
      beg = i + 1;
    }
    i++;
  }

  if (i) {
    auto r = trim(b, e, beg, i);
    if (r.first < r.second) { fn(&b[r.first], &b[r.second]); }
  }
}

inline bool is_hex(char c, int &v) {
  if (0x20 <= c && isdigit(c)) {
    v = c - '0';
    return true;
  } else if ('A' <= c && c <= 'F') {
    v = c - 'A' + 10;
    return true;
  } else if ('a' <= c && c <= 'f') {
    v = c - 'a' + 10;
    return true;
  }
  return false;
}

inline bool from_hex_to_i(const std::string &s, size_t i, size_t cnt,
                          int &val) {
  if (i >= s.size()) { return false; }

  val = 0;
  for (; cnt; i++, cnt--) {
    if (!s[i]) { return false; }
    auto v = 0;
    if (is_hex(s[i], v)) {
      val = val * 16 + v;
    } else {
      return false;
    }
  }
  return true;
}

inline size_t to_utf8(int code, char *buff) {
  if (code < 0x0080) {
    buff[0] = (code & 0x7F);
    return 1;
  } else if (code < 0x0800) {
    buff[0] = static_cast<char>(0xC0 | ((code >> 6) & 0x1F));
    buff[1] = static_cast<char>(0x80 | (code & 0x3F));
    return 2;
  } else if (code < 0xD800) {
    buff[0] = static_cast<char>(0xE0 | ((code >> 12) & 0xF));
    buff[1] = static_cast<char>(0x80 | ((code >> 6) & 0x3F));
    buff[2] = static_cast<char>(0x80 | (code & 0x3F));
    return 3;
  } else if (code < 0xE000) { // D800 - DFFF is invalid...
    return 0;
  } else if (code < 0x10000) {
    buff[0] = static_cast<char>(0xE0 | ((code >> 12) & 0xF));
    buff[1] = static_cast<char>(0x80 | ((code >> 6) & 0x3F));
    buff[2] = static_cast<char>(0x80 | (code & 0x3F));
    return 3;
  } else if (code < 0x110000) {
    buff[0] = static_cast<char>(0xF0 | ((code >> 18) & 0x7));
    buff[1] = static_cast<char>(0x80 | ((code >> 12) & 0x3F));
    buff[2] = static_cast<char>(0x80 | ((code >> 6) & 0x3F));
    buff[3] = static_cast<char>(0x80 | (code & 0x3F));
    return 4;
  }

  // NOTREACHED
  return 0;
}

std::string decode_url(const std::string &s,
                              bool convert_plus_to_space) {
  std::string result;

  for (size_t i = 0; i < s.size(); i++) {
    if (s[i] == '%' && i + 1 < s.size()) {
      if (s[i + 1] == 'u') {
        auto val = 0;
        if (from_hex_to_i(s, i + 2, 4, val)) {
          // 4 digits Unicode codes
          char buff[4];
          size_t len = to_utf8(val, buff);
          if (len > 0) { result.append(buff, len); }
          i += 5; // 'u0000'
        } else {
          result += s[i];
        }
      } else {
        auto val = 0;
        if (from_hex_to_i(s, i + 1, 2, val)) {
          // 2 digits hex codes
          result += static_cast<char>(val);
          i += 2; // '00'
        } else {
          result += s[i];
        }
      }
    } else if (convert_plus_to_space && s[i] == '+') {
      result += ' ';
    } else {
      result += s[i];
    }
  }

  return result;
}

void parse_query_text(const std::string &s, Params &params) {
  std::set<std::string> cache;
  split(s.data(), s.data() + s.size(), '&', [&](const char *b, const char *e) {
    std::string kv(b, e);
    if (cache.find(kv) != cache.end()) { return; }
    cache.insert(kv);

    std::string key;
    std::string val;
    split(b, e, '=', [&](const char *b2, const char *e2) {
      if (key.empty()) {
        key.assign(b2, e2);
      } else {
        val.assign(b2, e2);
      }
    });

    if (!key.empty()) {
      params.emplace(decode_url(key, true), decode_url(val, true));
    }
  });
}

bool compare_case_ignore(const std::string &a, const std::string &b) {
  if (a.size() != b.size()) { return false; }
  for (size_t i = 0; i < b.size(); i++) {
    if (::tolower(a[i]) != ::tolower(b[i])) { return false; }
  }
  return true;
}

bool has_crlf(const std::string &s) {
  auto p = s.c_str();
  while (*p) {
    if (*p == '\r' || *p == '\n') { return true; }
    p++;
  }
  return false;
}


// const char *get_header_value(const Headers &headers,
//                                     const std::string &key, size_t id,
//                                     const char *def) {
//   auto rng = headers.equal_range(key);
//   auto it = rng.first;
//   std::advance(it, static_cast<ssize_t>(id));
//   if (it != rng.second) { return it->second.c_str(); }
//   return def;
// }