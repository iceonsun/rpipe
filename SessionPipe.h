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
    static const IUINT32 MIN_EXPIRE_SEC = 10;
    static const IUINT32 MAX_EXPIRE_SEC = 120;

    // this two methods should be declared here. remove them from ipipe
    SessionPipe(IPipe *pipe, uv_loop_t *loop, const KeyType &key, const sockaddr_in *target);

    SessionPipe(IPipe *pipe, uv_loop_t *loop, int conv, const sockaddr_in *target);

    ~SessionPipe() override;

    virtual void SetExpireIfNoOps(IUINT32 sec);

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
    IUINT32 mCnt = 0;
    IUINT32 mLastCnt = 0;
    RTimer *mRepeatTimer = nullptr;
    uv_loop_t *mLoop;
};


#endif //RPIPE_SESSIONPIPE_H
