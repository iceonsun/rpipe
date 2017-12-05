//
// Created on 12/4/17.
//

#include <cassert>
#include <unistd.h>
#include <sys/errno.h>
#include <syslog.h>
#include "TcpRdWriter.h"
#include "FdUtil.h"
#include "rcommon.h"
#include "debug.h"


TcpRdWriter::TcpRdWriter(uv_stream_t *stream) {
    int fd;
    int err = uv_fileno(reinterpret_cast<const uv_handle_t *>(stream), &fd);
    assert(err == 0);
    do_init(stream, fd);
}

TcpRdWriter::TcpRdWriter(int fd, uv_loop_t *loop) {
    uv_tcp_t *tcp = static_cast<uv_tcp_t *>(malloc(sizeof(uv_tcp_t)));
    uv_tcp_init(loop, tcp);
    int ret = openStream(reinterpret_cast<uv_stream_t *>(tcp), fd);
    assert(ret == 0);

    do_init(reinterpret_cast<uv_stream_t *>(tcp), fd);
}

void TcpRdWriter::do_init(uv_stream_t *stream, int fd) {
    mStream = stream;
    mFd = fd;
    FdUtil::CheckStreamFd(mFd);
    int ret = FdUtil::SetNonblocking(mFd);
    assert(ret != -1);
}

int TcpRdWriter::Read(char *buf, int len, int *err) {
    assert(len > 0);
    ssize_t ret = read(mFd, buf, len);
    if (ret <= 0 && err) {
        *err = errno;
    }

    return ret;
}

int TcpRdWriter::Write(const char *data, int len, int *err) {
    assert(len > 0);
    if (len > 0) {
        rwrite_req_t *req = static_cast<rwrite_req_t *>(malloc(sizeof(rwrite_req_t)));
        req->buf.base = (char *) malloc(len);
        req->buf.len = len;
        req->write.data = this;
        memcpy(req->buf.base, data, len);
        uv_write(reinterpret_cast<uv_write_t *>(req), mStream, &req->buf, 1, write_cb);
    }
    return len;
}

TcpRdWriter::~TcpRdWriter() {
    assert(mFd == -1);
}

int TcpRdWriter::Close() {
    if (mStream) {
        debug(LOG_ERR, "Close()");
        uv_close(reinterpret_cast<uv_handle_t *>(mStream), close_cb);
        mFd = -1;
        mStream = nullptr;
    }
}

void TcpRdWriter::write_cb(uv_write_t *write, int status) {
    rwrite_req_t *req = reinterpret_cast<rwrite_req_t *>(write);
    free_rwrite_req(req);
    if (status) {
        debug(LOG_ERR, "write_cb err: %s", uv_strerror(status));
        TcpRdWriter *rdWriter = static_cast<TcpRdWriter *>(req->write.data);
        rdWriter->OnErr(status);
    }
}

int TcpRdWriter::openStream(uv_stream_t *stream, int fd) {
    return uv_tcp_open(reinterpret_cast<uv_tcp_t *>(stream), fd);
}

