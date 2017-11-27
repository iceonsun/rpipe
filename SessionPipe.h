//
// Created on 11/21/17.
//

#ifndef RPIPE_SESSIONPIPE_H
#define RPIPE_SESSIONPIPE_H


#include "IPipe.h"
#include "RTimer.h"
#include "ITopContainerPipe.h"

class SessionPipe : public ITopContainerPipe {
public:
    static const IUINT32 MIN_EXPIRE_SEC = 10;
    static const IUINT32 MAX_EXPIRE_SEC = 120;
    static const int HEAD_LEN = sizeof(IUINT32) + 1;

    typedef std::string KeyType;

    // this two methods should be declared here. remove them from ipipe
    SessionPipe(IPipe *pipe, uv_loop_t *loop, const KeyType &key, const sockaddr_in *target);
    SessionPipe(IPipe *pipe, uv_loop_t *loop, int conv, const sockaddr_in *target);

    virtual ~SessionPipe();

    KeyType GetKey();

    virtual void SetExpireIfNoOps(IUINT32 sec);

    int Send(ssize_t nread, const rbuf_t *buf) override;

    int Input(ssize_t nread, const rbuf_t *buf) override;

    // if no ops for last data input/output, pipe will close automatically.

    int Close() override;

    static int IsCloseSignal(ssize_t nread, const rbuf_t *buf);

    static inline KeyType BuildKey(ssize_t nread, const rbuf_t *rbuf);
    static inline KeyType BuildKey(int conv, const struct sockaddr_in *addr);

protected:
    static inline ssize_t insertHead(char *base, int len, char cmd, IUINT32 conv);

    static inline const char *decodeHead(const char *base, int len, char *cmd, IUINT32 *conv);

    void onPeerEof();
    void onSelfEof();

private:
    static int decodeConv(const KeyType &key);

    void timer_cb(void *arg);

    void timeoutToClose();

    void notifyPeerClose();

    int doSend(ssize_t nread, const rbuf_t *buf);

private:
    struct sockaddr_in *mAddr = nullptr;
    IUINT32 mCnt = 0;
    IUINT32 mLastCnt = 0;
    RTimer *mRepeatTimer = nullptr;
    KeyType mKey;
    int mConv;
    uv_loop_t *mLoop;

};


#endif //RPIPE_SESSIONPIPE_H
