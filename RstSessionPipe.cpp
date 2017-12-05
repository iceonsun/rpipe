//
// Created on 11/29/17.
//

#include <syslog.h>
#include "RstSessionPipe.h"
#include "debug.h"

RstSessionPipe::RstSessionPipe(IPipe *pipe, uv_loop_t *loop, const SessionPipe::KeyType &key, const sockaddr_in *target)
        : SessionPipe(pipe, loop, key, target) {}

int RstSessionPipe::Input(ssize_t nread, const rbuf_t *buf) {
    debug(LOG_ERR, "input");
    notifyPeerClose(RST);
    return UV_EOF;
}

int RstSessionPipe::Send(ssize_t nread, const rbuf_t *buf) {
    return 0;
}
