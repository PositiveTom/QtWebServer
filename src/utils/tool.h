#ifndef TOOL_H
#define TOOL_H

#include <iostream>

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



#endif