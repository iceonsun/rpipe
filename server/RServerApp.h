//
// Created on 11/17/17.
//

#ifndef RPIPE_RSERVER_H
#define RPIPE_RSERVER_H


#include "../RApp.h"

class RServerApp : public RApp {
public:
    bool isServer() override { return true; };

    BridgePipe *CreateBridgePipe(const Config &conf, IPipe *btmPipe, uv_loop_t *loop) override;

    IPipe *CreateBtmPipe(const Config &conf, uv_loop_t *loop) override;

    virtual ISessionPipe *OnRawData(const ISessionPipe::KeyType &key, const void *addr);

protected:
    virtual uv_udp_t *createBtmDgram(const Config &conf, uv_loop_t *loop);

    virtual ISessionPipe *CreateStreamPipe(int sock, const ISessionPipe::KeyType &key, const void *addr);
};


#endif //RPIPE_RSERVER_H
