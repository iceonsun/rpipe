//
// Created on 11/13/17.
//

#include <cassert>
#include "IPipe.h"

IPipe::~IPipe() {
    if (mAddr) {
        free(mAddr);
        mAddr = nullptr;
    }
}

int IPipe::Output(ssize_t nread, const rbuf_t *buf) {
//    if (mOutputCb) {
        return mOutputCb(nread, buf);
//    }
    return -1;
}

int IPipe::OnRecv(ssize_t nread, const rbuf_t *buf) {
    if (mOnRecvCb) {
        return mOnRecvCb(nread, buf);
    }
    return -1;
}

void IPipe::SetOutputCb(IPipe::PipeCb cb) {
    mOutputCb = cb;
}

void IPipe::SetOnRecvCb(IPipe::PipeCb cb) {
    mOnRecvCb = cb;
}

int IPipe::Send(ssize_t nread, const rbuf_t *buf) {
    return Output(nread, buf);
}

int IPipe::Input(ssize_t nread, const rbuf_t *buf) {
    return OnRecv(nread, buf);
}

void IPipe::OnError(IPipe *pipe, int err) {
    if (mErrCb) {
        mErrCb(pipe, err);
    }
}

void IPipe::SetOnErrCb(IPipe::ErrCb cb) {
    mErrCb = cb;
}
void IPipe::SetTargetAddr(const sockaddr_in *target) {
    if (!target) {
        fprintf(stderr, "target is nulll!\n");
        return;
    }

    if (mAddr) {
        fprintf(stderr, "cannot set addr twice!\n");
        return;
    }

    mAddr = static_cast<sockaddr_in *>(malloc(sizeof(struct sockaddr_in)));
    memcpy(mAddr, target, sizeof(struct sockaddr_in));
}
