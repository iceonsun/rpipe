//
// Created on 11/13/17.
//

#include <cassert>
#include <syslog.h>
#include <util.h>
#include "BtmDGramPipe.h"
#include "debug.h"


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
    if (mDgram) {
        uv_close(reinterpret_cast<uv_handle_t *>(mDgram), close_cb);
        mDgram->data = nullptr;
        free(mDgram);
        mDgram = nullptr;
    }

    return 0;
}

int BtmDGramPipe::Output(ssize_t nread, const rbuf_t *buf) {
    if (nread > 0 && buf->data) {
        auto req = (rudp_send_t *) malloc(sizeof(rudp_send_t));
        req->udp_send.data = this;
        req->buf = uv_buf_init(buf->base, nread);
        req->addr = static_cast<sockaddr *>(buf->data);

        struct sockaddr_in *addr = reinterpret_cast<sockaddr_in *>(req->addr);
//        debug(LOG_ERR, "req: %p. send %d bytes to %s:%d. curr: %d.", req, nread, inet_ntoa(addr->sin_addr),
//              ntohs(addr->sin_port), iclock() % 10000);

        uv_udp_send(reinterpret_cast<uv_udp_send_t *>(req), mDgram, &req->buf, 1,
                    reinterpret_cast<const sockaddr *>(req->addr), send_cb);
    }
    return 0;
}

void BtmDGramPipe::send_cb(uv_udp_send_t *req, int status) {
//    debug(LOG_ERR, "req: %p, status: %d, error: %s\n", req, status, status ? uv_strerror(status) : "");
    auto send = reinterpret_cast<rudp_send_t *>(req);
    auto pipe = (BtmDGramPipe *) send->udp_send.data;
    send->udp_send.data = nullptr;

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
        pipe->OnRecv(nread, &rbuf);
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


void BtmDGramPipe::SetOutputCb(const PipeCb &cb) {
    fprintf(stderr, "cannot set output of a btm terminal.\n");
}

int BtmDGramPipe::Input(ssize_t nread, const rbuf_t *buf) {
    fprintf(stderr, "cannot call input on a btm stream pipe\n");
}

void BtmDGramPipe::close_cb(uv_handle_t *handle) {
    free(handle);
}
