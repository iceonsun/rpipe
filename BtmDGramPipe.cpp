//
// Created on 11/13/17.
//

#include "BtmDGramPipe.h"


BtmDGramPipe::BtmDGramPipe(uv_udp_t *dgram) {
    mDgram = dgram;
}


int BtmDGramPipe::Init() {
    mDgram->data = this;
    uv_udp_recv_start(mDgram, alloc_buf, recv_cb);

    return 0;
}


int BtmDGramPipe::Close() {
    if (mDgram) {
        uv_close(reinterpret_cast<uv_handle_t *>(mDgram), close_cb);
        mDgram->data = nullptr;
        free(mDgram);
//        free(mAddr);
        mDgram = nullptr;
    }

//    if (mAddr) {
//        free(mAddr);
//    }

    return 0;
}

int BtmDGramPipe::Output(ssize_t nread, const rbuf_t *buf) {
    if (nread > 0 && buf->data) {
        auto req = (rudp_send_t *) malloc(sizeof(rudp_send_t));
        req->udp_send.data = this;
        req->buf = uv_buf_init(buf->base, nread);
        struct sockaddr *addr = static_cast<sockaddr *>(buf->data);

        memcpy(req->buf.base, buf->base, buf->len);
        uv_udp_send(reinterpret_cast<uv_udp_send_t *>(req), mDgram, &req->buf, 1,
                    addr, send_cb);
    }
    return 0;
}

void BtmDGramPipe::send_cb(uv_udp_send_t *req, int status) {
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
    int nret = nread;
    if (nread > 0) {
        rbuf_t rbuf = {0};
        rbuf.base = buf->base;
        rbuf.len = nread;
        rbuf.data = (void *) addr;
        nret = pipe->OnRecv(nread, &rbuf);
    }

    free(buf->base);
    if (nret < 0) {
        pipe->OnError(pipe, nret);
    }
}

int BtmDGramPipe::Send(ssize_t nread, const rbuf_t *buf) {
    if (nread > 0) {
        rbuf_t rbuf = {0};
        rbuf.base = (char *) malloc(nread);
        rbuf.len = nread;
        rbuf.data = buf->data;  // this stores addr
        memcpy(rbuf.base, buf->base, nread);    // cp data to this pipe
        int nret = Output(nread, &rbuf);
//        free(rbuf.base);  // it's freed in output
        if (nret < 0) {
            OnError(this, nret);
        }
        return nret;
    }
    return 0;
}

void BtmDGramPipe::SetOutputCb(IPipe::PipeCb cb) {
    fprintf(stderr, "cannot set output of a btm terminal.\n");
}

int BtmDGramPipe::Input(ssize_t nread, const rbuf_t *buf) {
    fprintf(stderr, "cannot call input on a btm stream pipe\n");
}

void BtmDGramPipe::close_cb(uv_handle_t *handle) {
    free(handle);
}
