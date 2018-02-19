//
// Created on 11/15/17.
//

#ifndef RPIPE_RCLIENT_H
#define RPIPE_RCLIENT_H


#include "../RApp.h"
#include "../BtmDGramPipe.h"
#include "../BridgePipe.h"

// RClient/RServer uses udp

class RClientApp : public RApp {
public:
    bool isServer() override { return false; };

    int Init() override;

    void Close() override;

    BridgePipe *CreateBridgePipe(const Config &conf, IPipe *btmPipe, uv_loop_t *loop) override;

    IPipe *CreateBtmPipe(const Config &conf, uv_loop_t *loop) override;

    virtual ISessionPipe *OnRawData(const ISessionPipe::KeyType &key, const void *addr);

protected:
    virtual int initTcpListen(const Config &conf, uv_loop_t *loop);

private:
    static const int A_RRIME = 6173;

    void onNewClient(uv_stream_t *client);

    void newConn(uv_stream_t *server, int status);

    static void svr_conn_cb(uv_stream_t *server, int status);

private:
    uint32_t mConv = 0;
    uv_handle_t *mListenHandle = nullptr;
};


#endif //RPIPE_RCLIENT_H
