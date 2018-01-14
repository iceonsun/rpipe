//
// Created on 11/28/17.
//

#ifndef RPIPE_RAWBTMPIPE_H
#define RPIPE_RAWBTMPIPE_H

#include "uv.h"
#include "../IPipe.h"

class BtmPipe : public IPipe {
public:
    // will set fd to blocking mode.
    BtmPipe(int fd, uv_loop_t *loop);
    virtual ~BtmPipe();

    int Init() override;

    int Close() override;

    int GetFd();

protected:
    virtual int outputCb(ssize_t nread, const rbuf_t *buf) = 0;
    virtual int onReadable(uv_poll_t *handle) = 0;

protected:
    static void pollcb(uv_poll_t* handle, int status, int events);

private:
    uv_poll_t* mPoll = nullptr;
    int mFd = -1;
    uv_loop_t *mLoop = nullptr;
};


#endif //RPIPE_RAWBTMPIPE_H
