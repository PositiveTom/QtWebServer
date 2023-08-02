#ifndef SERVERMSG_H
#define SERVERMSG_H

#include <string>

/*基类*/
struct IpMsg {
    std::string ip;
    uint16_t port;
};

/*具体类*/
struct Ipv4Msg : public IpMsg {};


#endif