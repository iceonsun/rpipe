//
// Created on 11/17/17.
//

#ifndef RPIPE_RSERVER_H
#define RPIPE_RSERVER_H


#include "../RApp.h"

// todo: when target tcp is closed. we should notify peer to close.
class RServer : public RApp {
public:
    int Loop(Config &conf) override;

    bool isServer() override { return true; };

    void Close() override;

    void Flush() override;

    BridgePipe *CreateBridgePipe(const Config &conf) override;

    IPipe *CreateBtmPipe(const Config &conf) override;

    virtual ISessionPipe * OnRawData(const ISessionPipe::KeyType &key, const void *addr);

protected:
    virtual uv_udp_t *CreateBtmDgram(const Config &conf);

private:
    virtual ISessionPipe *CreateStreamPipe(int sock, const ISessionPipe::KeyType &key, const void *addr);

private:
    uv_loop_t *mLoop = nullptr;
    BridgePipe *mBrigde = nullptr;
    uv_timer_t mFlushTimer;
    Config mConf;
    struct sockaddr_in mTargetAddr;
    IUINT32 mConv;  // all connection shares a single conv
};


#endif //RPIPE_RSERVER_H
