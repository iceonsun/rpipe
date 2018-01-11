//
// Created on 11/29/17.
//

#include "RstSessionPipe.h"

RstSessionPipe::RstSessionPipe(IPipe *pipe, const KeyType &key, const sockaddr_in *target)
        : ISessionPipe(pipe, key, target) {}

int RstSessionPipe::Input(ssize_t nread, const rbuf_t *buf) {
    notifyPeerClose(RST);
    return UV_EOF;
}

int RstSessionPipe::Send(ssize_t nread, const rbuf_t *buf) {
    return 0;
}