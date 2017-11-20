//
// Created on 11/20/17.
//

#include "EchoBtmPipe.h"

int EchoBtmPipe::Init() {
    return 0;
}

int EchoBtmPipe::Send(ssize_t nread, const rbuf_t *buf) {
    if (nread > 0) {
        rbuf_t *rbuf = static_cast<rbuf_t *>(malloc(sizeof(rbuf_t)));
        rbuf->base = static_cast<char *>(malloc(nread));
        memcpy(rbuf->base, buf->base, nread);
        rbuf->len = nread;
        rbuf->data = buf->data; // it's addr will not be freed
        mPendingData.push_back(rbuf);
    }
    return nread;
}

int EchoBtmPipe::Close() {
    for (auto buf: mPendingData) {
        free_rbuf(buf);
    }
    mPendingData.clear();
    return 0;
}

void EchoBtmPipe::Flush(IUINT32 curr) {
    for (auto buf: mPendingData) {
        OnRecv(buf->len, buf);
        free_rbuf(buf);
    }
    mPendingData.clear();
}
