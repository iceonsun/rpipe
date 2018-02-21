//
// Created on 11/13/17.
//

#include <cassert>
#include <cstring>
#include <plog/Log.h>
#include "TopStreamPipe.h"

TopStreamPipe::TopStreamPipe(uv_stream_t *stream, uint32_t mss) {
    mTopStream = stream;
    mMSS = mss;
}

int TopStreamPipe::Init() {
    int n = uv_read_start(mTopStream, alloc_buf, read_cb);
    mTopStream->data = this;
    assert(mMSS > 0);
    return n;
}

// lower -> Pipe
int TopStreamPipe::Input(ssize_t nread, const rbuf_t *buf) {
    assert(nread > 0);

    if (nread > 0) {
        rwrite_req_t *req = static_cast<rwrite_req_t *>(malloc(sizeof(rwrite_req_t)));
        req->buf.base = (char *) malloc(nread);
        req->buf.len = nread;
        memcpy(req->buf.base, buf->base, nread);

        req->write.data = this;
        uv_write(reinterpret_cast<uv_write_t *>(req), mTopStream, &req->buf, 1, write_cb);
        return nread;
    }

    return nread;
}


int TopStreamPipe::Close() {
    IPipe::Close();

    if (mTopStream) {   // todo: delay close
        uv_close(reinterpret_cast<uv_handle_t *>(mTopStream), close_cb);
        mTopStream->data = nullptr;
        mTopStream = nullptr;
    }
    return 0;
}

void TopStreamPipe::read_cb(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf) {
    auto pipe = (TopStreamPipe *) stream->data;

    rbuf_t rbuf = {0};
    rbuf.base = buf->base;
    rbuf.len = nread;

    if (nread > 0) {
        rbuf_t tmp;
        tmp.base = buf->base;
        tmp.len = buf->len;

        ssize_t nleft = nread;
        while (nleft > 0) {
            ssize_t n = nleft;
            if (n > pipe->mMSS) {
                n = pipe->mMSS;
            }
            tmp.base = buf->base + (nread - nleft);
            nleft -= n;
            pipe->Output(n, &tmp);
        }
    } else {
        pipe->Output(nread, &rbuf);
    }

    free(rbuf.base);
}

void TopStreamPipe::write_cb(uv_write_t *uvreq, int status) {
    rwrite_req_t *req = reinterpret_cast<rwrite_req_t *>(uvreq);
    auto pipe = static_cast<TopStreamPipe *> (req->write.data);
    req->write.data = nullptr;

    free_rwrite_req(req);
    LOGV_IF(status == UV_ECANCELED) << "stream write canceled";
    if (status < 0 && UV_ECANCELED != status) {
        pipe->OnError(pipe, status);
    }
}

int TopStreamPipe::Send(ssize_t nread, const rbuf_t *buf) {
    LOGE << "cannot call send on a top stream pipe";
    return -1;
}

int TopStreamPipe::OnRecv(ssize_t nread, const rbuf_t *buf) {
    LOGE << "cannot call onrecv on a top stream pipe";
    return -1;
}
