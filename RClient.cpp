//
// Created on 11/15/17.
//

#include <uv.h>
#include <cstdlib>
#include <nmq.h>
#include <sstream>
#include <enc.h>
#include "RClient.h"
#include "TopStreamPipe.h"
#include "NMQPipe.h"
#include "UdpBtmPipe.h"
#include "RstSessionPipe.h"
#include "TcpRdWriter.h"


RClient::~RClient() {
}

int RClient::Loop(Config &conf) {
//    mConv = iclock() % A_RRIME;   todo: add this later
    conf.param.localListenPort = 10010;
    conf.param.localListenIface = "127.0.0.1";

    conf.param.targetIp = "127.0.0.1";
    conf.param.targetPort = 10011;
//    conf.param.targetIp = "47.95.217.247";
    mLoop = uv_default_loop();
    int nret = uv_ip4_addr(conf.param.targetIp, conf.param.targetPort, &mTargetAddr);
    if (nret != 0) {
        fprintf(stderr, "wrong conf: %s\n", uv_strerror(nret));
        return -1;
    }

    mListenHandle = InitTcpListen(conf);
    if (!mListenHandle) {
        fprintf(stderr, "failed to listen tcp\n");
        return 1;
    }

    mBridge = CreateBridgePipe(conf);

    uv_timer_init(mLoop, &mFlushTimer);

    mFlushTimer.data = this;
    uv_timer_start(&mFlushTimer, flush_cb, conf.param.interval, conf.param.interval);
    mBridge->Start();
    uv_run(mLoop, UV_RUN_DEFAULT);

    Close();
    return 0;
}

//IPipe *RClient::CreateBtmPipe(const Config &conf) {
//    uv_udp_t *dgram = static_cast<uv_udp_t *>(malloc(sizeof(uv_udp_t)));
//    uv_udp_init(mLoop, dgram);
//
//    return new BtmDGramPipe(dgram);
//}

IPipe *RClient::CreateBtmPipe(const Config &conf) {
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    return new UdpBtmPipe(sock, mLoop);
}

void RClient::Close() {
    if (mBridge) {
        mBridge->SetOnCreateNewPipeCb(nullptr);
        mBridge->SetOnErrCb(nullptr);

        mBridge->Close();
        delete mBridge;
        mBridge = nullptr;
    }

    if (mListenHandle) {
        uv_close(mListenHandle, close_cb);
        mListenHandle = nullptr;
    }
}

uv_handle_t *RClient::InitTcpListen(const Config &conf) {
    uv_tcp_t *tcp = static_cast<uv_tcp_t *>(malloc(sizeof(uv_tcp_t)));
    tcp->data = this;
    uv_tcp_init(mLoop, tcp);

    struct sockaddr_in addr = {0};
    uv_ip4_addr(conf.param.localListenIface, conf.param.localListenPort, &addr);
    int nret = uv_tcp_bind(tcp, reinterpret_cast<const sockaddr *>(&addr), 0);
    if (0 == nret) {
        nret = uv_listen(reinterpret_cast<uv_stream_t *>(tcp), conf.param.BACKLOG, svr_conn_cb);
        if (nret) {
            debug(LOG_ERR, "failed to listen tcp: %s\n", uv_strerror(nret));
            uv_close(reinterpret_cast<uv_handle_t *>(tcp), close_cb);
        }
    } else {
        debug(LOG_ERR, "failed to bind %s:%d, error: %s\n", conf.param.localListenIface, conf.param.localListenPort,
              uv_strerror(nret));
        free(tcp);
    }

    if (nret) {
        debug(LOG_ERR, "failed to start, err: %s\n", uv_strerror(nret));
        tcp = nullptr;
    } else {
        debug(LOG_ERR, "client, listening on tcp %s:%d", conf.param.localListenIface, conf.param.localListenPort);
        debug(LOG_ERR, "target udp %s:%d", conf.param.targetIp, conf.param.targetPort);
    }

    return reinterpret_cast<uv_handle_t *>(tcp);
}

void RClient::svr_conn_cb(uv_stream_t *server, int status) {
    RClient *rc = static_cast<RClient *>(server->data);
    rc->newConn(server, status);
}

void RClient::newConn(uv_stream_t *server, int status) {
    debug(LOG_ERR, "new connection. status: %d\n", status);
    if (status) {
        debug(LOG_ERR, "new connection error. status: %d, err: %s\n", status, uv_strerror(status));
        return;
    }

    uv_tcp_t *client = static_cast<uv_tcp_t *>(malloc(sizeof(uv_tcp_t)));
    uv_stream_t *stream = reinterpret_cast<uv_stream_t *>(client);
    uv_tcp_init(mLoop, client);
    if (uv_accept(server, stream) == 0) {
        onNewClient(stream);
    } else {
        uv_close(reinterpret_cast<uv_handle_t *>(client), close_cb);
    }
}

void RClient::onNewClient(uv_stream_t *client) {
    mConv++;
//    IPipe *top = new TopStreamPipe(client);

    IRdWriter *rdWriter = new TcpRdWriter(client);
    IPipe *nmq = new NMQPipe(mConv, rdWriter);   // nmq pipe addr
    SessionPipe *sess = new SessionPipe(nmq, mLoop, mConv, &mTargetAddr);
    sess->SetExpireIfNoOps(20); // todo: fill value from conf

    mBridge->AddPipe(sess);
}

BridgePipe *RClient::CreateBridgePipe(const Config &conf) {
    IPipe *btmPipe = CreateBtmPipe(conf);
    if (!btmPipe) {
        fprintf(stderr, "failed to start.\n");
        return nullptr;
    }

    auto *pipe = new BridgePipe(btmPipe);
    auto fn = std::bind(&RClient::OnRawData, this, std::placeholders::_1, std::placeholders::_2);
    pipe->SetOnCreateNewPipeCb(fn);    // explicitly set cb. ignore unknown data

    pipe->SetOnErrCb([this](IPipe *pipe, int err) {
        fprintf(stderr, "bridge pipe error: %d. Exit!\n", err);
        uv_stop(this->mLoop);
    });
    pipe->Init();
    return pipe;
}

void RClient::Flush() {
    mBridge->Flush(iclock());
}

SessionPipe *RClient::OnRawData(const SessionPipe::KeyType &key, const void *addr) {
    SessionPipe *sess = new RstSessionPipe(nullptr, nullptr, key, static_cast<const sockaddr_in *>(addr));
    return sess;
}

