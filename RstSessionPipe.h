//
// Created on 11/29/17.
//

#ifndef RPIPE_RSTSESSIONPIPE_H
#define RPIPE_RSTSESSIONPIPE_H


#include "SessionPipe.h"

class RstSessionPipe : public SessionPipe {
public:
    RstSessionPipe(IPipe *pipe, uv_loop_t *loop, const KeyType &key, const sockaddr_in *target);

    int Input(ssize_t nread, const rbuf_t *buf) override;

    int Send(ssize_t nread, const rbuf_t *buf) override;
};


#endif //RPIPE_RSTSESSIONPIPE_H
