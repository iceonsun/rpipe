//
// Created on 11/13/17.
//

#include <syslog.h>
#include "IPipe.h"

IPipe::~IPipe() {
#ifndef RPIPE_NNDEBUG
    assert(mOutputCb == nullptr);
    assert(mOnRecvCb == nullptr);
    assert(mErrCb == nullptr);
#endif
}

int IPipe::Output(ssize_t nread, const rbuf_t *buf) {
    if (mOutputCb) {
        return mOutputCb(nread, buf);
    }
    return -1;
}

int IPipe::OnRecv(ssize_t nread, const rbuf_t *buf) {
    if (mOnRecvCb) {
        return mOnRecvCb(nread, buf);
    }
    return -1;
}

void IPipe::SetOutputCb(const PipeCb &cb) {
    mOutputCb = cb;
}

void IPipe::SetOnRecvCb(const PipeCb &cb) {
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

void IPipe::SetOnErrCb(const ErrCb &cb) {
    mErrCb = cb;
}

int IPipe::Close() {
    mOutputCb = nullptr;
    mOnRecvCb = nullptr;
    mErrCb = nullptr;
}

