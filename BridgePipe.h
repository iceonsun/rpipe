//
// Created on 11/18/17.
//

#ifndef RPIPE_IBRIDGEPIPE_H
#define RPIPE_IBRIDGEPIPE_H

#include <list>
#include <map>
#include "IPipe.h"
#include "ISessionPipe.h"
#include "util/Handler.h"

class BridgePipe : public IPipe {
public:
    typedef std::function<ISessionPipe *(const ISessionPipe::KeyType &, const void *addr)> OnCreatePipeCb;

    explicit BridgePipe(IPipe *btmPipe, uv_loop_t *loop);

    /* inherited from IPipe */
    int Init() override;

    int Input(ssize_t nread, const rbuf_t *buf) override;

    int Close() override;

    void Flush(uint32_t curr) override;

    // top pipes should call this function to send data
    virtual int PSend(ISessionPipe *pipe, ssize_t nread, const rbuf_t *buf);

    /* new methods */
    // if (key) must be true

    virtual int AddPipe(ISessionPipe *pipe);

    virtual int RemovePipe(ISessionPipe *pipe);

    virtual ISessionPipe *FindPipe(const ISessionPipe::KeyType &key) const;

    virtual void OnTopPipeError(ISessionPipe *pipe, int err);

    virtual void OnBtmPipeError(IPipe *pipe, int err);

    virtual void SetOnCreateNewPipeCb(const OnCreatePipeCb &cb);

protected:
    virtual ISessionPipe *onCreateNewPipe(const ISessionPipe::KeyType &key, void *addr);

private:

    int doRemove(std::map<ISessionPipe::KeyType, ISessionPipe *>::iterator it);

    int removeAll();

    void cleanErrPipes();

    void handleMessage(const Handler::Message &message);

private:
    static const int MSG_CLOSE_PIPE = 0;

    std::map<ISessionPipe::KeyType, ISessionPipe *> mTopPipes;
    std::map<ISessionPipe::KeyType, ISessionPipe *> mErrPipes;
    IPipe *mBtmPipe;
    OnCreatePipeCb mCreateNewPipeCb;
    uv_loop_t *mLoop = nullptr;
    Handler::SPHandler mHandler;
};


#endif //RPIPE_IBRIDGEPIPE_H
