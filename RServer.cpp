//
// Created on 11/17/17.
//

#include <cstdio>
#include <cassert>

#include <sstream>

#include <unistd.h>

#include "util.h"
#include "RServer.h"
#include "BtmDGramPipe.h"
#include "NMQPipe.h"
#include "TopStreamPipe.h"

int RServer::Loop(Config &conf) {
    conf.param.localListenIface = "127.0.0.1";
    conf.param.localListenPort = 10011;

    conf.param.targetIp = "127.0.0.1";
    conf.param.targetPort = 10022;

    mLoop = uv_default_loop();
    mConf = conf;

    uv_ip4_addr(conf.param.targetIp, conf.param.targetPort, &mTargetAddr);

    mBrigde = CreateBridgePipe(conf);

    uv_timer_init(mLoop, &mFlushTimer);

    mFlushTimer.data = this;
    uv_timer_start(&mFlushTimer, flush_cb, conf.param.interval * 2, conf.param.interval);

    uv_run(mLoop, UV_RUN_DEFAULT);
}

void RServer::Close() {
    if (mBrigde) {
        mBrigde->SetHashRawDataFunc(nullptr);
        mBrigde->SetOnFreshDataCb(nullptr);
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
    bridge->SetOnFreshDataCb([this](ssize_t nread, const rbuf_t *buf) -> IPipe * {
        return this->OnRawData(nread, buf);
    });
    bridge->SetHashRawDataFunc(HashKeyFunc);
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

uv_udp_t *RServer::CreateBtmDgram(const Config &conf) {
    uv_udp_t *udp = static_cast<uv_udp_t *>(malloc(sizeof(uv_udp_t)));
    uv_udp_init(mLoop, udp);
    struct sockaddr_in addr = {0};
    uv_ip4_addr(conf.param.localListenIface, conf.param.localListenPort, &addr);
    int nret = uv_udp_bind(udp, (const struct sockaddr *) &addr, UV_UDP_REUSEADDR);
    if (nret) {
        fprintf(stderr, "bind error: %s\n", uv_strerror(nret));
        free(udp);
        return nullptr;
    }
    udp->data = this;
    return udp;
}

IPipe *RServer::OnRawData(ssize_t nread, const rbuf_t *buf) {
    if (nread > 0 && buf->data) {
        int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        // do this synchronously
        int nret = connect(sock, reinterpret_cast<const sockaddr *>(&mTargetAddr), sizeof(mTargetAddr));
        if (nret) {
            fprintf(stderr, "failed to connect: %s\n", strerror(errno));
            return nullptr;
        }
        uv_tcp_t *tcp = static_cast<uv_tcp_t *>(malloc(sizeof(uv_tcp_t)));
        uv_tcp_init(mLoop, tcp);
        nret = uv_tcp_open(tcp, sock);
        if (nret) {
            free(tcp);
            close(sock);
        }
        return CreateStreamPipe(reinterpret_cast<uv_stream_t *>(tcp), nread, buf);

    }
    return nullptr;
}

IPipe *RServer::CreateStreamPipe(uv_stream_t *conn, ssize_t nread, const rbuf_t *rbuf) {
    IPipe *top = new TopStreamPipe(conn);
    NMQPipe *nmq = new NMQPipe(++mConv, top);
    struct sockaddr_in *addr = static_cast<sockaddr_in *>(malloc(sizeof(struct sockaddr_in)));
    memcpy(addr, rbuf->data, sizeof(addr));
    nmq->SetTargetAddr(addr);
//    nmq->Init();

    debug(LOG_INFO, "nmq pipe: %p", nmq);
    auto key = HashKeyFunc(nread, rbuf);
    assert(!key.empty());
    mBrigde->AddPipe(key, nmq);
    return nmq;
}

BridgePipe::KeyType RServer::HashKeyFunc(ssize_t nread, const rbuf_t *buf) {
    if (nread > sizeof(IUINT32) && buf->data && buf->base) {
        IUINT32 conv = nmq_get_conv(buf->base);
        auto *addr = static_cast<sockaddr_in *>(buf->data);
        std::ostringstream out;
//        out << addr->sin_addr.s_addr << ":" << addr->sin_port;
        out << inet_ntoa(addr->sin_addr) << ":" << ntohs(addr->sin_port) << ":" << conv;
        return out.str();
    }

    return nullptr;
}
