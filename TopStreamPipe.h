//
// Created on 11/13/17.
//

#ifndef RPIPE_TOPSTREAMPIPE_H
#define RPIPE_TOPSTREAMPIPE_H


#include <uv.h>
#include "IPipe.h"

class TopStreamPipe : public IPipe {
public:
    TopStreamPipe(uv_stream_t *stream);

    int Init() override;

//    int OnRecv(ssize_t nread, const uv_buf_t *buf) override;

    int Input(ssize_t nread, const rbuf_t *buf) override;

    // invalid
    int Send(ssize_t nread, const rbuf_t *buf) override;

    // invalid
    int OnRecv(ssize_t nread, const rbuf_t *buf) override;

    int Close() override;

protected:
    static void echo_read(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf);

    static void write_cb(uv_write_t *uvreq, int status);

private:
    uv_stream_t *mTopStream;
};


#endif //RPIPE_TOPSTREAMPIPE_H