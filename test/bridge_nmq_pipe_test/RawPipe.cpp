//
// Created on 11/20/17.
//

#include <syslog.h>
#include "RawPipe.h"
#include "../../debug.h"

int RawPipe::OnRecv(ssize_t nread, const rbuf_t *buf) {
    if (nread > 0) {
        printf("recv %d bytes: %.*s\n", nread, nread, buf->base);
    } else {
        debug(LOG_ERR, "failed to recv");
    }
    return nread;
}

int RawPipe::Init() {
    return 0;
}

int RawPipe::Close() {
    return 0;
}
