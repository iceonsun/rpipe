//
// Created on 11/28/17.
//

#ifndef RPIPE_RAWBTMPIPE_H
#define RPIPE_RAWBTMPIPE_H


#include "IPipe.h"

class BtmPipe : public IPipe {
public:
    // will set fd to blocking mode.
    BtmPipe(int fd, uv_loop_t *loop);
    virtual ~BtmPipe();

    int Close() override;

    int GetFd();

protected:
    virtual int outputCb(ssize_t nread, const rbuf_t *buf) = 0;
    virtual int onReadable(uv_poll_t *handle) = 0;

protected:
    static void pollcb(uv_poll_t* handle, int status, int events);

private:
    uv_poll_t mPoll;
    int mFd;
};


#endif //RPIPE_RAWBTMPIPE_H
