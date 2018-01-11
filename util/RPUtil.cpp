//
// Created on 1/11/18.
//

#include <plog/Log.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include "RPUtil.h"

std::string RPUtil::Addr2Str(const struct sockaddr *addr) {
    if (!addr) {
        return "";
    } else if (addr->sa_family == AF_INET) {
        struct sockaddr_in *addr4 = (struct sockaddr_in *) addr;
        std::string s = inet_ntoa(addr4->sin_addr);
        s += ":";
        s += htons(addr4->sin_port);
        return s;
    } else if (addr->sa_family == AF_UNIX) {
        struct sockaddr_un *un = (struct sockaddr_un *) addr;
        return un->sun_path;
    }
    LOGE << "Unsupported protocol: " << addr->sa_family;
    return "";
}
