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
    static const int HEAD_LEN = sizeof(uint32_t) + 1;

    explicit ISessionPipe(IPipe *topPipe, const KeyType &key, const sockaddr_in *target);

    ~ISessionPipe() override;

    int Close() override;

    KeyType GetKey();

    enum CMD {
        NOP = 0,    // normal data flow
        SYN = 1,
        FIN = 2,    // close normally
        RST = 3,    // peer conn donsn't exist
    };

public:
    static int IsCloseSignal(ssize_t nread, const rbuf_t *buf);

    static KeyType BuildKey(uint32_t conv, const struct sockaddr_in *addr);

    // if declared static inline, cannot compile
    static KeyType BuildKey(ssize_t nread, const rbuf_t *rbuf);

    static uint32_t ConvFromKey(const KeyType &key);


protected:
    static ssize_t insertHead(char *base, int len, char cmd, uint32_t conv);

    static const char *decodeHead(const char *base, int len, char *cmd, uint32_t *conv);

    static uint32_t decodeConv(const KeyType &key);

    void notifyPeerClose(char cmd = FIN);

protected:
    KeyType mKey;
    struct sockaddr_in *mAddr = nullptr;
    uint32_t mConv = 0;
};


#endif //RPIPE_ISESSIONPIPE_H
