//
// Created on 11/13/17.
//

#include <cstdio>
#include <cassert>
#include <plog/Log.h>
#include "BtmDGramPipe.h"
#include "util/RPUtil.h"


BtmDGramPipe::BtmDGramPipe(uv_udp_t *dgram) {
    mDgram = dgram;
}

int BtmDGramPipe::Init() {
    IPipe::Init();
    mDgram->data = this;
    uv_udp_recv_start(mDgram, alloc_buf, recv_cb);

    return 0;
}

int BtmDGramPipe::Close() {
    IPipe::Close();
    if (mDgram) {
        mDgram->data = nullptr;
        uv_close(reinterpret_cast<uv_handle_t *>(mDgram), close_cb);
        mDgram = nullptr;
    }

    return 0;
}

int BtmDGramPipe::Output(ssize_t nread, const rbuf_t *buf) {
    if (nread > 0 && buf->data) {
        auto req = (rudp_send_t *) malloc(sizeof(rudp_send_t));
        memset(req, 0, sizeof(rudp_send_t));

        req->udp_send.data = this;
        req->buf = uv_buf_init(buf->base, nread);   // base and data are malloced by self
        req->addr = static_cast<struct sockaddr *>(buf->data);

        uv_udp_send(reinterpret_cast<uv_udp_send_t *>(req), mDgram, &req->buf, 1,
                    (const struct sockaddr*)req->addr, send_cb);
    }
    return 0;
}

void BtmDGramPipe::send_cb(uv_udp_send_t *req, int status) {
    auto send = reinterpret_cast<rudp_send_t *>(req);
    auto pipe = (BtmDGramPipe *) send->udp_send.data;
    send->udp_send.data = nullptr;

    LOGV << "send " << send->buf.len << " bytes to " << RPUtil::Addr2Str(send->addr);
    free_rudp_send(send);

    if (status) {
        pipe->OnError(pipe, status);
    }
}


void BtmDGramPipe::recv_cb(uv_udp_t *handle, ssize_t nread, const uv_buf_t *buf, const struct sockaddr *addr,
                           unsigned flags) {
    BtmDGramPipe *pipe = static_cast<BtmDGramPipe *>(handle->data);
    if (nread > 0) {
        rbuf_t rbuf = {0};
        rbuf.base = buf->base;
        rbuf.len = nread;
        rbuf.data = (void *) addr;
        pipe->Input(nread, &rbuf);
    }   // todo nret should be check here!!
    else if (nread < 0) {
        pipe->OnError(pipe, nread);
    }

    free(buf->base);
}

int BtmDGramPipe::Send(ssize_t nread, const rbuf_t *buf) {
    assert(nread > 0);

    int nret = nread;
    if (nread > 0) {
        struct sockaddr_in *addr = static_cast<sockaddr_in *>(malloc(sizeof(struct sockaddr_in)));
        memcpy(addr, buf->data, sizeof(struct sockaddr_in));

        rbuf_t rbuf = {0};
        rbuf.base = (char *) malloc(nread);
        rbuf.len = nread;
        rbuf.data = addr;  // this stores addr
        memcpy(rbuf.base, buf->base, nread);    // cp data to this pipe
        nret = Output(nread, &rbuf);
    }
    return nret;
}