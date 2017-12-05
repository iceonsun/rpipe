//
// Created on 11/28/17.
//

#include <unistd.h>
#include <syslog.h>
#include "BtmPipe.h"
#include "debug.h"
#include "FdUtil.h"

int BtmPipe::Close() {
    if (mFd >= 0) {
        uv_poll_stop(&mPoll);
        close(mFd);
        mFd = -1;
    }
}

BtmPipe::BtmPipe(int fd, uv_loop_t *loop) : mFd(fd) {
    FdUtil::CheckDgramFd(fd);

    FdUtil::SetNonblocking(mFd);    // todo: check if this is necessary or better place

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
