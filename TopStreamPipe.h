//
// Created on 11/13/17.
//

#ifndef RPIPE_TOPSTREAMPIPE_H
#define RPIPE_TOPSTREAMPIPE_H


#include <uv.h>
#include "IPipe.h"

class TopStreamPipe : public IPipe {
public:
    explicit TopStreamPipe(uv_stream_t *stream, uint32_t mss);

    int Init() override;

    int Input(ssize_t nread, const rbuf_t *buf) override;

    int Send(ssize_t nread, const rbuf_t *buf) override;

    int OnRecv(ssize_t nread, const rbuf_t *buf) override;

    int Close() override;

protected:
    static void read_cb(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf);

    static void write_cb(uv_write_t *uvreq, int status);

private:
    uv_stream_t *mTopStream = nullptr;
    uint32_t mMSS = 0;
};


#endif //RPIPE_TOPSTREAMPIPE_H