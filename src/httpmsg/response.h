#ifndef RESPONSE_H
#define RESPONSE_H

#include <string>
#include <httpmsg/request.h>
#include <utils/tool.h>

struct Response {
    std::string version;
    Headers headers;
    int status=-1;
    void set_header(const std::string &key,
                                 const std::string &val) {
        if (!has_crlf(key) && !has_crlf(val)) {
            headers.emplace(key, val);
        }
    }
};



#endif