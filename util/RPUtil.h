//
// Created on 1/11/18.
//

#ifndef RPIPE_RPUTIL_H
#define RPIPE_RPUTIL_H

#include <string>

class RPUtil {
public:
    static std::string Addr2Str(const struct sockaddr *addr);
};


#endif //RPIPE_RPUTIL_H
