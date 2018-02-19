//
// Created on 11/21/17.
//

#ifndef RPIPE_SESSIONPIPE_H
#define RPIPE_SESSIONPIPE_H


#include "IPipe.h"
#include "util/RTimer.h"
#include "ITopContainerPipe.h"
#include "ISessionPipe.h"

class SessionPipe : public ISessionPipe {
public:
    static const uint32_t MIN_EXPIRE_SEC = 10;
    static const uint32_t MAX_EXPIRE_SEC = 120;

    // this two methods should be declared here. remove them from ipipe
    SessionPipe(IPipe *pipe, uv_loop_t *loop, const KeyType &key, const sockaddr_in *target);

    SessionPipe(IPipe *pipe, uv_loop_t *loop, uint32_t conv, const sockaddr_in *target);

    virtual void SetExpireIfNoOps(uint32_t sec);

    int Send(ssize_t nread, const rbuf_t *buf) override;

    int Input(ssize_t nread, const rbuf_t *buf) override;

    // if no ops for last data input/output, pipe will close automatically.

    int Close() override;

protected:

    void onPeerEof();

    void onSelfEof();

    void onPeerRst();

    void timeoutToClose();

private:
    void timer_cb(void *arg);

    int doSend(ssize_t nread, const rbuf_t *buf);

private:
    uint32_t mCnt = 0;
    uint32_t mLastCnt = 0;
    RTimer *mRepeatTimer = nullptr;
    uv_loop_t *mLoop = nullptr;
};


#endif //RPIPE_SESSIONPIPE_H
