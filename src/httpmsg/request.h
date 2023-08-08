#ifndef REQUEST_H
#define REQUEST_H

#include <string>
#include <map>
// #include <utils/tool.h>

struct ci {
  bool operator()(const std::string &s1, const std::string &s2) const {
    return std::lexicographical_compare(s1.begin(), s1.end(), s2.begin(),
                                        s2.end(),
                                        [](unsigned char c1, unsigned char c2) {
                                          return ::tolower(c1) < ::tolower(c2);/*位于全局命名空间,域名解析符不用加命名空间,转换成小写字母*/
                                        });
  }
};
using Headers = std::multimap<std::string, std::string, ci>;
using Params = std::multimap<std::string, std::string>;

struct Request {
    std::string version;
    std::string target;
    std::string method;

    std::string path;
    Params params;

    Headers headers;

    std::string get_header_value(const std::string &key,
                                             size_t id) const {
        // const char *get_header_value(const Headers &headers,
        //                                     const std::string &key, size_t id,
        //                                     const char *def) {
            auto rng = headers.equal_range(key);
            auto it = rng.first;
            std::advance(it, static_cast<ssize_t>(id));
            if (it != rng.second) { return it->second.c_str(); }
            return "";
            // return def;
        // }
        // return get_header_value(headers, key, id, "");
    }
};


#endif