//
// Created on 11/15/17.
//

#include <uv.h>
#include <cstdlib>
#include <nmq/nmq.h>
#include <plog/Log.h>

#include "RClientApp.h"
#include "../TopStreamPipe.h"
#include "../SessionPipe.h"
#include "../RstSessionPipe.h"
//#include "../bio/UdpBtmPipe.h"
#include "../nmq/NMQPipe.h"

int RClientApp::Init() {
    int nret = RApp::Init();
    if (nret) {
        return nret;
    }

    mLoop = GetLoop();
    nret = initTcpListen(GetConfig(), mLoop);
    return nret;
}

void RClientApp::Close() {
    if (mListenHandle) {
        uv_close(mListenHandle, close_cb);
        mListenHandle = nullptr;
    }
    RApp::Close();
}

IPipe *RClientApp::CreateBtmPipe(const Config &conf, uv_loop_t *loop) {
    uv_udp_t *dgram = static_cast<uv_udp_t *>(malloc(sizeof(uv_udp_t)));
    uv_udp_init(loop, dgram);

    return new BtmDGramPipe(dgram);
}

// synchronous send
//IPipe *RClientApp::CreateBtmPipe(const Config &conf, uv_loop_t *loop) {
//    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
//    return new UdpBtmPipe(sock, loop);
//}

BridgePipe *RClientApp::CreateBridgePipe(const Config &conf, IPipe *btmPipe, uv_loop_t *loop) {
    auto *pipe = new BridgePipe(btmPipe, loop);
    auto fn = std::bind(&RClientApp::OnRawData, this, std::placeholders::_1, std::placeholders::_2);
    pipe->SetOnCreateNewPipeCb(fn);    // explicitly set cb. ignore unknown data

    pipe->SetOnErrCb([loop](IPipe *pipe, int err) {
        LOGE << "bridge pipe error: "<< err << ". Exit!";
        uv_stop(loop);
    });
    return pipe;
}

int RClientApp::initTcpListen(const Config &conf, uv_loop_t *loop) {
    uv_tcp_t *tcp = static_cast<uv_tcp_t *>(malloc(sizeof(uv_tcp_t)));
    tcp->data = this;
    uv_tcp_init(loop, tcp);

    struct sockaddr_in addr = {0};
    uv_ip4_addr(conf.param.localListenIface.c_str(), conf.param.localListenPort, &addr);
    int nret = uv_tcp_bind(tcp, reinterpret_cast<const sockaddr *>(&addr), 0);
    if (0 == nret) {
        nret = uv_listen(reinterpret_cast<uv_stream_t *>(tcp), conf.param.backlog, svr_conn_cb);
        if (nret) {
            LOGE << "failed to listen tcp: " << uv_strerror(nret);
            uv_close(reinterpret_cast<uv_handle_t *>(tcp), close_cb);
        }
    } else {
        LOGE << "failed to bind " << conf.param.localListenIface << ":" << conf.param.localListenPort << ", error: "
             << uv_strerror(nret);
        free(tcp);
    }

    if (nret) {
        tcp = nullptr;
    } else {
        LOGD << "client, listening on tcp " << conf.param.localListenIface << ":" << conf.param.localListenPort;
        LOGD << "target udp " << conf.param.targetIp << ":" << conf.param.targetPort;
        mListenHandle = reinterpret_cast<uv_handle_t *>(tcp);
    }

    return nret;
}

void RClientApp::svr_conn_cb(uv_stream_t *server, int status) {
    RClientApp *rc = static_cast<RClientApp *>(server->data);
    rc->newConn(server, status);
}

void RClientApp::newConn(uv_stream_t *server, int status) {
    LOGV << "new connection. status: " << status;
    if (status) {
        LOGE << "new connection error: " << uv_strerror(status);
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

void RClientApp::onNewClient(uv_stream_t *client) {
    mConv++;
    auto &conf = GetConfig();
    IPipe *top = new TopStreamPipe(client, GET_NMQ_MTU());
    auto nmq = NewNMQPipeFromConf(mConv, conf, top);
    nmq->SetSteady(false);
    SessionPipe *sess = new SessionPipe(nmq, mLoop, mConv, GetTarget());
    sess->SetExpireIfNoOps(20);

    GetBridgePipe()->AddPipe(sess);
}

ISessionPipe *RClientApp::OnRawData(const ISessionPipe::KeyType &key, const void *addr) {
    ISessionPipe *sess = new RstSessionPipe(nullptr, key, static_cast<const sockaddr_in *>(addr));
    return sess;
}
