#ifndef REQUEST_H
#define REQUEST_H

#include <iostream>
#include <cstring>
#include <string>
#include <map>
#include <set>
#include <functional>

struct header_sort {
    bool operator()(const std::string& s1, const std::string& s2) const {
        return std::lexicographical_compare(s1.begin(), s1.end(), s2.begin(), s2.end(), [](u_char c1, u_char c2){
            return ::tolower(c1) < ::tolower(c2);
        });
    }
};

using Headers = std::multimap<std::string, std::string, header_sort>;

struct Request {
    /*http报文的 请求行*/
    std::string m_method;
    std::string m_target; //url
    std::string m_version;

    /*以下为请求行的潜在附加信息*/
    std::string m_path; //请求行中的url可以会附带路径 ?后面的  url ? path

    /*http报文的 首部字段*/
    Headers m_headers;
};

bool parse_request_line(const char *s, Request &req);
bool is_space_or_tab(char c);
bool compare_case_ignore(const std::string &a, const std::string &b);
std::string decode_url(const std::string &s,
                              bool convert_plus_to_space);


template <typename T>
bool parse_header(const char *beg, const char *end, T fn) {/*解析http报文中的首部字段,key-value的格式*/
  // Skip trailing spaces and tabs.
  while (beg < end && is_space_or_tab(end[-1])) {
    end--;
  }

  auto p = beg;
  while (p < end && *p != ':') {
    p++;
  }

  if (p == end) { return false; }

  auto key_end = p;

  if (*p++ != ':') { return false; }/*在运行++之前,*p应该等于":",让p指向下一个字符"*/

  while (p < end && is_space_or_tab(*p)) {
    p++;
  }
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

#endif