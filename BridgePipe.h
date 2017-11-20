//
// Created on 11/18/17.
//

#ifndef RPIPE_IBRIDGEPIPE_H
#define RPIPE_IBRIDGEPIPE_H

#include <list>
#include <map>
#include "IPipe.h"

class BridgePipe : public IPipe {
public:
    explicit BridgePipe(IPipe *btmPipe);

    typedef std::string KeyType;

    typedef std::function<KeyType(ssize_t nread, const rbuf_t *buf)> HashFunc;
    typedef std::function<IPipe *(ssize_t nread, const rbuf_t *buf)> OnFreshDataCb;

    /* inherited from IPipe */
    int Init() override;

    int Input(ssize_t nread, const rbuf_t *buf) override;

    int Close() override;

    void Flush(IUINT32 curr) override;

    // top pipes should call this function
    virtual int PSend(IPipe *pipe, ssize_t nread, const rbuf_t *buf);

    /* new methods */
    // if (key) must be true

    virtual void SetOnFreshDataCb(OnFreshDataCb cb);

    virtual void SetHashRawDataFunc(HashFunc fn);

    virtual int AddPipe(BridgePipe::KeyType key, IPipe *pipe);

    virtual int RemovePipe(IPipe *pipe);

    virtual IPipe *FindPipe(ssize_t nread, const rbuf_t *buf);

    virtual void OnTopPipeError(IPipe *pipe, int err);

    virtual void OnBtmPipeError(IPipe *pipe, int err);

protected:
    virtual IPipe *onFreshData(ssize_t nread, const rbuf_t *buf);

    virtual BridgePipe::KeyType keyForRawData(ssize_t nread, const rbuf_t *data);

private:

    int doRemove(std::map<KeyType, IPipe *>::iterator it);

    int removeAll();

    inline void cleanUseless();

    std::map<KeyType, struct sockaddr *> mAddrMap;
    std::map<KeyType, IPipe *> mTopPipes;
    IPipe *mBtmPipe;
    HashFunc mHashFunc;
    std::list<IPipe *> mUselessPipe;
    OnFreshDataCb mFreshDataCb;
};


#endif //RPIPE_IBRIDGEPIPE_H
