//
// Created on 11/17/17.
//

#include <cstdio>
#include <cassert>

#include <sstream>

#include <unistd.h>

#include "FdUtil.h"
#include "RServer.h"
#include "BtmDGramPipe.h"
#include "NMQPipe.h"
#include "TopStreamPipe.h"
#include "UdpBtmPipe.h"
#include "RstSessionPipe.h"
#include "TcpRdWriter.h"

int RServer::Loop(Config &conf) {
    conf.param.localListenIface = "0.0.0.0";
    conf.param.localListenPort = 10011;

    conf.param.targetIp = "127.0.0.1";
    conf.param.targetPort = 10022;

    mLoop = uv_default_loop();
    mConf = conf;

    uv_ip4_addr(conf.param.targetIp, conf.param.targetPort, &mTargetAddr);

    mBrigde = CreateBridgePipe(conf);

    uv_timer_init(mLoop, &mFlushTimer);

    mFlushTimer.data = this;
    uv_timer_start(&mFlushTimer, flush_cb, conf.param.interval, conf.param.interval);

    mBrigde->Start();
    uv_run(mLoop, UV_RUN_DEFAULT);
    Close();
}

void RServer::Close() {
    if (mBrigde) {
        mBrigde->SetOnCreateNewPipeCb(nullptr);
        mBrigde->SetOnErrCb(nullptr);
        mBrigde->Close();
        delete mBrigde;
        mBrigde = nullptr;
    }
}

void RServer::Flush() {
    mBrigde->Flush(iclock());
}

BridgePipe *RServer::CreateBridgePipe(const Config &conf) {
    IPipe *pipe = CreateBtmPipe(conf);
    auto *bridge = new BridgePipe(pipe);

    bridge->SetOnErrCb([this](IPipe *pipe, int err) {
//        uv_timer_stop(&this->mFlushTimer);    // todo don't exit.
        fprintf(stderr, "bridge pipe error: %d. Exit!\n", err);
        uv_stop(this->mLoop);
    });
    auto fn = std::bind(&RServer::OnRawData, this, std::placeholders::_1, std::placeholders::_2);
//    bridge->SetOnCreateNewPipeCb([this](const SessionPipe::KeyType& key, const void *addr) -> SessionPipe * {
//        return this->OnRawData(key, addr);
//    });
    bridge->SetOnCreateNewPipeCb(fn);

    bridge->Init();
    return bridge;
}

IPipe *RServer::CreateBtmPipe(const Config &conf) {
    uv_udp_t *udp = CreateBtmDgram(conf);   // todo: check if udp is null
    if (udp) {
        IPipe *pipe = new BtmDGramPipe(udp);
        return pipe;
    }
    return nullptr;
}

//IPipe *RServer::CreateBtmPipe(const Config &conf) {
//    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
//    struct sockaddr_in addr;
//    uv_ip4_addr(mConf.param.localListenIface, mConf.param.localListenPort, &addr);
//    int n = bind(sock, reinterpret_cast<const sockaddr *>(&addr), sizeof(addr));
//    if (n) {
//        debug(LOG_ERR, "bind failed %s", strerror(n));
//        assert(n == 0);
//    }
//    return new UdpBtmPipe(sock, mLoop);
//}

uv_udp_t *RServer::CreateBtmDgram(const Config &conf) {
    uv_udp_t *udp = static_cast<uv_udp_t *>(malloc(sizeof(uv_udp_t)));
    uv_udp_init(mLoop, udp);
    struct sockaddr_in addr = {0};
    uv_ip4_addr(conf.param.localListenIface, conf.param.localListenPort, &addr);
    debug(LOG_ERR, "server, listening on udp: %s:%d", conf.param.localListenIface, conf.param.localListenPort);
    debug(LOG_ERR, "target tcp, %s:%d", conf.param.targetIp, conf.param.targetPort);

    int nret = uv_udp_bind(udp, (const struct sockaddr *) &addr, UV_UDP_REUSEADDR);
    if (nret) {
        fprintf(stderr, "bind error: %s\n", uv_strerror(nret));
        free(udp);
        return nullptr;
    }
    udp->data = this;
    return udp;
}

SessionPipe *RServer::OnRawData(const SessionPipe::KeyType &key, const void *addr) {
    if (addr) {
        int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        // do this synchronously. todo: do this asynchronously. create a data pool for pending data.
        int nret = connect(sock, reinterpret_cast<const sockaddr *>(&mTargetAddr), sizeof(mTargetAddr));
        if (nret) {
            SessionPipe *sess = new RstSessionPipe(nullptr, nullptr, key, static_cast<const sockaddr_in *>(addr));
            fprintf(stderr, "failed to connect %s: %d: %s\n", inet_ntoa(mTargetAddr.sin_addr),
                    ntohs(mTargetAddr.sin_port), strerror(nret));
            return sess;
        }
        return CreateStreamPipe(sock, key, addr);
    }
    return nullptr;
}

SessionPipe *RServer::CreateStreamPipe(int sock, const SessionPipe::KeyType &key, const void *arg) {
    IRdWriter *rdWriter = new TcpRdWriter(sock, mLoop);
    // bug here. mConv should match conv in key
    IUINT32 conv = SessionPipe::ConvFromKey(key);
    NMQPipe *nmq = new NMQPipe(conv, rdWriter);
    const struct sockaddr_in *addr = static_cast<const sockaddr_in *>(arg);
    SessionPipe *sess = new SessionPipe(nmq, mLoop, key, addr);

    debug(LOG_ERR, "sess pipe: %p", sess);
//    mBrigde->AddPipe(sess);
    return sess;
}

