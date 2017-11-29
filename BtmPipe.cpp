//
// Created on 11/28/17.
//

#include <unistd.h>
#include <syslog.h>
#include "BtmPipe.h"
#include "debug.h"

int BtmPipe::Close() {
    if (mFd >= 0) {
        uv_poll_stop(&mPoll);
        close(mFd);
        mFd = -1;
    }
}

BtmPipe::BtmPipe(int fd, uv_loop_t *loop) : mFd(fd) {
    CheckDgramFd(fd);

    SetNonblocking(mFd);    // todo: check if this is necessary or better place

    uv_poll_init(loop, &mPoll, mFd);
    mPoll.data = this;
    uv_poll_start(&mPoll, UV_READABLE, pollcb);

    auto out = std::bind(&BtmPipe::outputCb, this, std::placeholders::_1, std::placeholders::_2);
    SetOutputCb(out);
}

int BtmPipe::GetFd() {
    return mFd;
}

BtmPipe::~BtmPipe() {
    assert(mFd == -1);
}

void BtmPipe::pollcb(uv_poll_t *handle, int status, int events) {
    if (status) {
        debug(LOG_ERR, "poll wrong. status: %d, err: %s", status, uv_strerror(status));
        return;
    }

    if (events & UV_READABLE) {
        BtmPipe *pipe = static_cast<BtmPipe *>(handle->data);
        pipe->onReadable(handle);
    }
}

void BtmPipe::SetBlocking(int &fd) {
    int oflag = fcntl(fd, F_GETFL, 0);
    int newFlag = oflag & (~O_NONBLOCK);
    fcntl(fd, F_SETFL, newFlag);
}

void BtmPipe::SetNonblocking(int &fd) {
    int oflag = fcntl(fd, F_GETFL, 0);
    int newFlag = oflag | O_NONBLOCK;
    fcntl(fd, F_SETFL, newFlag);
}

void BtmPipe::CheckDgramFd(int fd) {
    assert(fd >= 0);
    int type;
    socklen_t len;
    getsockopt(fd, SOL_SOCKET, SO_TYPE, &type, &len);
    assert(type == SOCK_DGRAM);
}

