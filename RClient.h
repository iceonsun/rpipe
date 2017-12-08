//
// Created on 11/15/17.
//

#ifndef RPIPE_RCLIENT_H
#define RPIPE_RCLIENT_H


#include "RApp.h"
#include "BtmDGramPipe.h"
#include "BridgePipe.h"

// RClient/RServer uses udp

class RClient : public RApp {
public:
    virtual ~RClient();

    bool isServer() override { return false; };

    int Loop(Config &conf) override;

    void Close() override;

    void Flush() override;

    BridgePipe *CreateBridgePipe(const Config &conf) override;

    IPipe *CreateBtmPipe(const Config &conf) override;
    virtual ISessionPipe * OnRawData(const ISessionPipe::KeyType &key, const void *addr);
protected:

    virtual uv_handle_t *InitTcpListen(const Config &conf);

//    uv_handle_t *CreateListenHandle(const Config &conf) override;

private:
    static const int A_RRIME = 6173;

    virtual void onNewClient(uv_stream_t *client);

    static void svr_conn_cb(uv_stream_t *server, int status);

    void newConn(uv_stream_t *server, int status);

private:
    IUINT32 mConv = 0;
    uv_loop_t *mLoop;
    BridgePipe *mBridge = nullptr;
    uv_handle_t *mListenHandle = nullptr;
    uv_timer_t mFlushTimer;
    struct sockaddr_in mTargetAddr;
};


#endif //RPIPE_RCLIENT_H
