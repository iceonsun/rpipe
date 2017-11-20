//
// Created on 11/17/17.
//

#ifndef RPIPE_RSERVER_H
#define RPIPE_RSERVER_H


#include "RApp.h"

class RServer : public RApp {
public:
    int Loop(Config &conf) override;

    bool isServer() { return true; };

    void Close() override;

    void Flush() override;

    BridgePipe *CreateBridgePipe(const Config &conf) override;

    IPipe *CreateBtmPipe(const Config &conf) override;

    virtual IPipe *OnRawData(ssize_t nread, const rbuf_t *buf);

    static BridgePipe::KeyType HashKeyFunc(ssize_t nread, const rbuf_t *buf);

protected:
    virtual uv_udp_t *CreateBtmDgram(const Config &conf);

private:
    virtual IPipe *CreateTcpPipe(uv_stream_t *conn, ssize_t nread, const rbuf_t *rbuf);

private:
    uv_loop_t *mLoop;
    uv_handle_t *mListenHandle;
    BridgePipe *mBrigde;
    uv_timer_t mFlushTimer;
    Config mConf;
    struct sockaddr_in mTargetAddr;
    IUINT32 mConv;  // all connection shares a single conv
};


#endif //RPIPE_RSERVER_H
