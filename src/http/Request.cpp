#include <http/Request.h>

bool is_space_or_tab(char c) { return c == ' ' || c == '\t'; }

std::pair<size_t, size_t> trim(const char *b, const char *e, size_t left,
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

/**
 * @brief 解析请求行
*/
bool parse_request_line(const char *s, Request &req) {
  auto len = strlen(s);
  if (len < 2 || s[len - 2] != '\r' || s[len - 1] != '\n') { return false; }
  len -= 2;

  {
    size_t count = 0;

    split(s, s + len, ' ', [&](const char *b, const char *e) {/*把GET / HTTP/1.1 拆分开, 以' '为分隔符*/
      switch (count) {
      case 0: req.m_method = std::string(b, e); break;/*GET*/
      case 1: req.m_target = std::string(b, e); break;/*/*/
      case 2: req.m_version = std::string(b, e); break;/*HTTP/1.1*/
      default: break;
      }
      count++;
    });

    if (count != 3) { return false; }
  }

  static const std::set<std::string> methods{
      "GET",     "HEAD",    "POST",  "PUT",   "DELETE",
      "CONNECT", "OPTIONS", "TRACE", "PATCH", "PRI"};

  if (methods.find(req.m_method) == methods.end()) { return false; }

  if (req.m_version != "HTTP/1.1" && req.m_version != "HTTP/1.0") { return false; }

  {
    // Skip URL fragment /*它是用来标识文档中的特定位置或锚点的。当一个 URL 中包含片段时，浏览器在获取资源后，会尝试自动滚动到 URL 指定的片段所在位置。这样，可以方便地定位到文档的某个特定部分，例如一个标题、段落、图片、表格等等。*/
    for (size_t i = 0; i < req.m_target.size(); i++) {
      if (req.m_target[i] == '#') {
        req.m_target.erase(i);
        break;
      }
    }

    // size_t count = 0;

    // split(req.m_target.data(), req.m_target.data() + req.m_target.size(), '?',
    //               [&](const char *b, const char *e) {
    //                 switch (count) {
    //                 case 0:
    //                   req.m_path = decode_url(std::string(b, e), false);
    //                   break;
    //                 case 1: {
    //                   if (e - b > 0) {
    //                     parse_query_text(std::string(b, e), req.params);
    //                   }
    //                   break;
    //                 }
    //                 default: break;
    //                 }
    //                 count++;
    //               });/*在url中,?后面的是参数,前面的是路径,拆分url*/

    // if (count > 2) { return false; }
  }

  return true;
}

bool compare_case_ignore(const std::string &a, const std::string &b) {
  if (a.size() != b.size()) { return false; }
  for (size_t i = 0; i < b.size(); i++) {
    if (::tolower(a[i]) != ::tolower(b[i])) { return false; }
  }
  return true;
}

bool is_hex(char c, int &v) {
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


bool from_hex_to_i(const std::string &s, size_t i, size_t cnt,
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

size_t to_utf8(int code, char *buff) {
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

// template <typename T>
// bool parse_header(const char *beg, const char *end, T fn) {/*解析http报文中的首部字段,key-value的格式*/
//   // Skip trailing spaces and tabs.
//   while (beg < end && is_space_or_tab(end[-1])) {
//     end--;
//   }

//   auto p = beg;
//   while (p < end && *p != ':') {
//     p++;
//   }

//   if (p == end) { return false; }

//   auto key_end = p;

//   if (*p++ != ':') { return false; }/*在运行++之前,*p应该等于":",让p指向下一个字符"*/

//   while (p < end && is_space_or_tab(*p)) {
//     p++;
//   }
//   if (p < end) {
//     auto key = std::string(beg, key_end);
//     auto val = compare_case_ignore(key, "Location")
//                    ? std::string(p, end)
//                    : decode_url(std::string(p, end), false);
//     fn(std::move(key), std::move(val));
//     return true;
//   }

//   return false;
// }
