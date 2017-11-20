//
// Created on 11/13/17.
//

#ifndef RPIPE_IPIPE_H
#define RPIPE_IPIPE_H


#include <functional>
#include <types.h>
#include "rcommon.h"

#define PIPE_BUFSIZ 1500

class IPipe {
public:
    typedef std::function<int(ssize_t nread, const rbuf_t *buf)> PipeCb;
    typedef std::function<void(IPipe *pipe, int err)> ErrCb;

    virtual ~IPipe();

    virtual int Init() = 0;

    // buf.base within a pipe are reused. base from different pipes are copied.
    // so,  pipe should be responsable to free mem allocated by itself
    // pipe should not reuse base or buf passed by other pipes.
    // all methods should follow above rules.

    // data in. pass data to Output by default.
    virtual int Send(ssize_t nread, const rbuf_t *buf);   // todo: Send is called by other. Send默认调用Output.

    // data in. pass data to OnRecv by default.
    virtual int Input(ssize_t nread, const rbuf_t *buf);  // called by other

    // data out. data flows out passively
    virtual int
    OnRecv(ssize_t nread, const rbuf_t *buf); // todo: 按理说recv应该调用input。 recv 应该改成DataOut, 主动式。called by self

    // data out. calls onOutputCb by default.
    virtual int Output(ssize_t nread, const rbuf_t *buf); // output is called by self.

    virtual void SetOutputCb(PipeCb cb);

    virtual void SetOnRecvCb(PipeCb cb);

    virtual void OnError(IPipe *pipe, int err);

    virtual void SetOnErrCb(ErrCb cb);

    virtual int Close() = 0;

    virtual void Flush(IUINT32 curr) {}

    virtual void SetTargetAddr(const sockaddr_in *target);

    virtual struct sockaddr_in *GetTargetAddr() { return mAddr; }

protected:
    PipeCb mOutputCb = nullptr;
    PipeCb mOnRecvCb = nullptr;
    ErrCb mErrCb = nullptr;
    struct sockaddr_in *mAddr = nullptr;
};


#endif //RPIPE_IPIPE_H
