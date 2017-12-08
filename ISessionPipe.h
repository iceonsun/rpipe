//
// Created on 12/8/17.
//

#ifndef RPIPE_ISESSIONPIPE_H
#define RPIPE_ISESSIONPIPE_H


#include "ITopContainerPipe.h"
#include "IPipe.h"

class ISessionPipe : public ITopContainerPipe {
public:
    typedef std::string KeyType;
    static const int HEAD_LEN = sizeof(IUINT32) + 1;

    explicit ISessionPipe(IPipe *topPipe, const KeyType& key, const sockaddr_in *target);

    int Close() override;

    KeyType GetKey();

public:
    static int IsCloseSignal(ssize_t nread, const rbuf_t *buf);

    static KeyType BuildKey(int conv, const struct sockaddr_in *addr);

    // if declared static inline, cannot compile
    static KeyType BuildKey(ssize_t nread, const rbuf_t *rbuf);

    static IUINT32 ConvFromKey(const KeyType &key);

    ~ISessionPipe() override;

protected:
    static ssize_t insertHead(char *base, int len, char cmd, IUINT32 conv);

    static const char *decodeHead(const char *base, int len, char *cmd, IUINT32 *conv);

    static int decodeConv(const KeyType &key);

    void notifyPeerClose(char cmd = FIN);

protected:
    KeyType mKey;
    struct sockaddr_in *mAddr = nullptr;
    int mConv;
};


#endif //RPIPE_ISESSIONPIPE_H
