//
// Created on 11/17/17.
//

#include <cstdio>
#include <cassert>

#include <sstream>

#include <iostream>
#include <util.h>
#include <plog/Log.h>

#include "RServer.h"
#include "../BtmDGramPipe.h"
#include "../TopStreamPipe.h"
#include "../RstSessionPipe.h"
#include "../SessionPipe.h"
#include "../NMQPipe.h"
#include "../util/RPUtil.h"

int RServer::Loop(uv_loop_t *loop, Config &conf) {
    mLoop = loop;
    mConf = conf;

    uv_ip4_addr(conf.param.targetIp.c_str(), conf.param.targetPort, &mTargetAddr);

    mBrigde = CreateBridgePipe(conf);

    uv_timer_init(mLoop, &mFlushTimer);

    mFlushTimer.data = this;
    uv_timer_start(&mFlushTimer, flush_cb, conf.param.interval, conf.param.interval);

    mBrigde->Start();
    uv_run(mLoop, UV_RUN_DEFAULT);
    Close();
    return 0;
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
        fprintf(stderr, "bridge pipe error: %d. Exit!\n", err);
        uv_stop(this->mLoop);
    });
    auto fn = std::bind(&RServer::OnRawData, this, std::placeholders::_1, std::placeholders::_2);
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
    uv_ip4_addr(conf.param.localListenIface.c_str(), conf.param.localListenPort, &addr);
    LOGD << "server, listening on udp: " << conf.param.localListenIface << ":" << conf.param.localListenPort;
    LOGD << "target tcp " << conf.param.targetIp << ":" << conf.param.targetPort;

    int nret = uv_udp_bind(udp, (const struct sockaddr *) &addr, UV_UDP_REUSEADDR);
    if (nret) {
        LOGE << "uv_bind error: " << uv_strerror(nret);
        free(udp);
        return nullptr;
    }
    udp->data = this;
    return udp;
}

ISessionPipe *RServer::OnRawData(const ISessionPipe::KeyType &key, const void *addr) {
    if (addr) {
        int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        // do this synchronously. todo: do this asynchronously. create a data pool for pending data.
        int nret = connect(sock, reinterpret_cast<const sockaddr *>(&mTargetAddr), sizeof(mTargetAddr));
        if (nret) {
            ISessionPipe *sess = new RstSessionPipe(nullptr, key, static_cast<const sockaddr_in *>(addr));
            LOGE << "failed to connect " << RPUtil::Addr2Str((const struct sockaddr *) addr) << ", err: "
                 << strerror(errno);
            return sess;
        }
        return CreateStreamPipe(sock, key, addr);
    }
    return nullptr;
}

ISessionPipe *RServer::CreateStreamPipe(int sock, const ISessionPipe::KeyType &key, const void *arg) {
    uv_tcp_t *tcp = static_cast<uv_tcp_t *>(malloc(sizeof(uv_tcp_t)));
    uv_tcp_init(mLoop, tcp);
    int n = uv_tcp_open(tcp, sock);
    if (n) {
        free(tcp);
        LOGE << "uv_tcp_open failed: " << uv_strerror(n);
        return nullptr;
    }
    IPipe *top = new TopStreamPipe((uv_stream_t *) tcp);

    IUINT32 conv = ISessionPipe::ConvFromKey(key);
    IPipe *nmq = new NMQPipe(conv, top);
    const struct sockaddr_in *addr = static_cast<const sockaddr_in *>(arg);
    auto *sess = new SessionPipe(nmq, mLoop, key, addr);
    sess->SetExpireIfNoOps(20);
    return sess;
}

