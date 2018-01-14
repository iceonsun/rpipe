//
// Created on 11/17/17.
//

#include <cassert>

#include <util.h>
#include <plog/Log.h>
#include <nmq.h>

#include "RServerApp.h"
#include "../BtmDGramPipe.h"
#include "../TopStreamPipe.h"
#include "../RstSessionPipe.h"
#include "../SessionPipe.h"
#include "../util/RPUtil.h"
#include "../bio/UdpBtmPipe.h"

IPipe *RServerApp::CreateBtmPipe(const Config &conf, uv_loop_t *loop) {
    uv_udp_t *udp = createBtmDgram(conf, loop);
    if (udp) {
        IPipe *pipe = new BtmDGramPipe(udp);
        return pipe;
    }
    return nullptr;
}

//IPipe *RServerApp::CreateBtmPipe(const Config &conf, uv_loop_t *loop) {
//    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
//    struct sockaddr_in addr;
//    uv_ip4_addr(conf.param.localListenIface.c_str(), conf.param.localListenPort, &addr);
//    int n = bind(sock, reinterpret_cast<const sockaddr *>(&addr), sizeof(addr));
//    if (n) {
//        LOGE << "bind failed " << strerror(errno);
//        assert(n == 0);
//    }
//    return new UdpBtmPipe(sock, loop);
//}

BridgePipe *RServerApp::CreateBridgePipe(const Config &conf, IPipe *btmPipe, uv_loop_t *loop) {
    auto *bridge = new BridgePipe(btmPipe);

    bridge->SetOnErrCb([loop](IPipe *pipe, int err) {
        LOGE << "bridge pipe error: "<< err << ". Exit!";
        uv_stop(loop);
    });
    auto fn = std::bind(&RServerApp::OnRawData, this, std::placeholders::_1, std::placeholders::_2);
    bridge->SetOnCreateNewPipeCb(fn);

    return bridge;
}

uv_udp_t *RServerApp::createBtmDgram(const Config &conf, uv_loop_t *loop) {
    uv_udp_t *udp = static_cast<uv_udp_t *>(malloc(sizeof(uv_udp_t)));
    uv_udp_init(loop, udp);
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

ISessionPipe *RServerApp::OnRawData(const ISessionPipe::KeyType &key, const void *addr) {
    if (addr) {
        int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        // do this synchronously.
        int nret = connect(sock, reinterpret_cast<const sockaddr *>(GetTarget()), sizeof(*GetTarget()));
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

ISessionPipe *RServerApp::CreateStreamPipe(int sock, const ISessionPipe::KeyType &key, const void *arg) {
    uv_loop_t *loop = GetLoop();
    uv_tcp_t *tcp = static_cast<uv_tcp_t *>(malloc(sizeof(uv_tcp_t)));
    uv_tcp_init(loop, tcp);
    int n = uv_tcp_open(tcp, sock);
    if (n) {
        free(tcp);
        LOGE << "uv_tcp_open failed: " << uv_strerror(n);
        return nullptr;
    }
    auto &conf = GetConfig();
    IPipe *top = new TopStreamPipe((uv_stream_t *) tcp, conf.param.mtu - SEG_HEAD_SIZE);

    IUINT32 conv = ISessionPipe::ConvFromKey(key);
    auto nmq = NewNMQPipeFromConf(conv, conf, top);
    const struct sockaddr_in *addr = static_cast<const sockaddr_in *>(arg);
    auto *sess = new SessionPipe(nmq, loop, key, addr);
    sess->SetExpireIfNoOps(20);
    return sess;
}