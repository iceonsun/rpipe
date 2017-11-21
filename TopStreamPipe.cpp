//
// Created on 11/13/17.
//

#include <cassert>
#include <syslog.h>
#include "TopStreamPipe.h"
#include "debug.h"

TopStreamPipe::TopStreamPipe(uv_stream_t *stream) {
    mTopStream = stream;
}

int TopStreamPipe::Init() {
    uv_read_start(mTopStream, alloc_buf, echo_read);
    mTopStream->data = this;
    return 0;
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
    if (mTopStream) {
        uv_close(reinterpret_cast<uv_handle_t *>(mTopStream), close_cb);
        mTopStream->data = nullptr;
        mTopStream = nullptr;
    }
    return 0;
}

void TopStreamPipe::echo_read(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf) {
    auto pipe = (TopStreamPipe *) stream->data;

#ifndef NNDEBUG
    if (nread > 0) {
        debug(LOG_INFO, "read %d bytes: %.*s", nread, nread, buf);
    }
#endif

    rbuf_t rbuf = {0};
    rbuf.base = buf->base;
    rbuf.len = nread;

    pipe->Output(nread, &rbuf);
    free(rbuf.base);
}

void TopStreamPipe::close_cb(uv_handle_t *handle) {
    free(handle);
}

void TopStreamPipe::write_cb(uv_write_t *uvreq, int status) {
    rwrite_req_t *req = reinterpret_cast<rwrite_req_t *>(uvreq);
    auto pipe = static_cast<TopStreamPipe *> (req->write.data);
    req->write.data = nullptr;

    free_rwrite_req(req);
    if (status) {
        pipe->OnError(pipe, status);
    }
}

int TopStreamPipe::Send(ssize_t nread, const rbuf_t *buf) {
    fprintf(stderr, "cannot call send on a top stream pipe\n");
    return -1;
}

int TopStreamPipe::OnRecv(ssize_t nread, const rbuf_t *buf) {
    fprintf(stderr, "cannot call onrecv on a top stream pipe\n");
    return -1;
}



