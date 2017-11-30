//
// Created on 11/18/17.
//

#ifndef RPIPE_IBRIDGEPIPE_H
#define RPIPE_IBRIDGEPIPE_H

#include <list>
#include <map>
#include "IPipe.h"
#include "SessionPipe.h"

class BridgePipe : public IPipe {
public:
    typedef std::function<SessionPipe *(const SessionPipe::KeyType &, const void *addr)> OnCreatePipeCb;

    explicit BridgePipe(IPipe *btmPipe);

    /* inherited from IPipe */
    int Init() override;

    int Input(ssize_t nread, const rbuf_t *buf) override;

    int Close() override;

    void Flush(IUINT32 curr) override;

    // top pipes should call this function to send data
    virtual int PSend(SessionPipe *pipe, ssize_t nread, const rbuf_t *buf);

    /* new methods */
    // if (key) must be true

    virtual int AddPipe(SessionPipe *pipe);

    virtual int RemovePipe(SessionPipe *pipe);

    virtual SessionPipe *FindPipe(const SessionPipe::KeyType &key) const;

    virtual void OnTopPipeError(SessionPipe *pipe, int err);

    virtual void OnBtmPipeError(IPipe *pipe, int err);

    virtual void SetOnCreateNewPipeCb(const OnCreatePipeCb &cb);

    void Start() override;

protected:
    virtual SessionPipe *onCreateNewPipe(const SessionPipe::KeyType &key, void *addr);

private:

    int doRemove(std::map<SessionPipe::KeyType, SessionPipe *>::iterator it);

    int removeAll();

    inline void cleanErrPipes();

    std::map<SessionPipe::KeyType, SessionPipe *> mTopPipes;
    std::map<SessionPipe::KeyType, SessionPipe *> mErrPipes;
    IPipe *mBtmPipe;
    OnCreatePipeCb mCreateNewPipeCb;
};


#endif //RPIPE_IBRIDGEPIPE_H
