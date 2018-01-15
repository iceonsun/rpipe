//
// Created on 11/13/17.
//

#ifndef RPIPE_IPIPE_H
#define RPIPE_IPIPE_H

#include <cassert>
#include <functional>


#include "nmq/ktype.h"
#include "rcommon.h"

#define PIPE_BUFSIZ 1500

class IPipe {
public:
    typedef std::function<int(ssize_t nread, const rbuf_t *buf)> PipeCb;
    typedef std::function<void(IPipe *pipe, int err)> ErrCb;

    virtual ~IPipe();

    // todo: init & start
    virtual int Init() { mInited = true; };

    virtual void Start() { assert(mInited); }

    // buf.base within a pipe are reused. base from different pipes are copied.
    // so,  pipe should be responsable to free mem allocated by itself
    // pipe should not reuse base or buf passed by other pipes.
    // all methods should follow above rules.

    // data in. pass data to Output by default.
    virtual int Send(ssize_t nread, const rbuf_t *buf);

    // data in. pass data to OnRecv by default.
    // it's caller's responsibility to check validity of nread and buf.
    virtual int Input(ssize_t nread, const rbuf_t *buf);  // called by other

    // data out. data flows out passively
    // it's callee's responsibility to check if op succeeds and callee will signify error if any
    virtual int OnRecv(ssize_t nread, const rbuf_t *buf);

    // data out. calls onOutputCb by default.
    virtual int Output(ssize_t nread, const rbuf_t *buf); // output is called by self.

    virtual void SetOutputCb(const PipeCb &cb);

    virtual void SetOnRecvCb(const PipeCb &cb);

    // synchronous read/write will report error through return value
    // asynchronous read/write will report error through callback, which is OnError
    // @pipe is the nearest pipe to this pipe that can leads to error point.
    virtual void OnError(IPipe *pipe, int err);

    virtual void SetOnErrCb(const ErrCb &cb);

    virtual int Close();

    virtual void Flush(IUINT32 curr) {}

    // prevent copying
    IPipe &operator=(const IPipe &pipe) = delete;

protected:
    PipeCb mOutputCb = nullptr;
    PipeCb mOnRecvCb = nullptr;
    ErrCb mErrCb = nullptr;
//    struct sockaddr_in *mAddr = nullptr;
    bool mInited = false;
};


#endif //RPIPE_IPIPE_H
