//
// Created on 11/28/17.
//
#include <unistd.h>
#include <plog/Log.h>
#include "BtmPipe.h"
#include "../util/FdUtil.h"

int BtmPipe::Init() {
    IPipe::Init();

    mPoll = static_cast<uv_poll_t *>(malloc(sizeof(uv_poll_t)));
    uv_poll_init(mLoop, mPoll, mFd);
    mPoll->data = this;
    uv_poll_start(mPoll, UV_READABLE, pollcb);

    auto out = std::bind(&BtmPipe::outputCb, this, std::placeholders::_1, std::placeholders::_2);
    SetOutputCb(out);
    return 0;
}

int BtmPipe::Close() {
    if (mPoll) {
        uv_poll_stop(mPoll);
        uv_close(reinterpret_cast<uv_handle_t *>(mPoll), close_cb);
        close(mFd);
        mPoll = nullptr;
    }
    return 0;
}

BtmPipe::BtmPipe(int fd, uv_loop_t *loop) : mFd(fd) {
    FdUtil::CheckDgramFd(fd);

    FdUtil::SetNonblocking(mFd);    // todo: check if this is necessary or better place

    mLoop = loop;
}

int BtmPipe::GetFd() {
    return mFd;
}

BtmPipe::~BtmPipe() {
    assert(mFd == -1);
}

void BtmPipe::pollcb(uv_poll_t *handle, int status, int events) {
    if (status) {
        LOGE << "poll wrong. status: " << status << ", err: " << uv_strerror(status);
        return;
    }

    if (events & UV_READABLE) {
        auto *pipe = static_cast<BtmPipe *>(handle->data);
        pipe->onReadable(handle);
    }
}

