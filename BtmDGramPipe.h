//
// Created on 11/13/17.
//

#ifndef RPIPE_BTMDGRAMPIPE_H
#define RPIPE_BTMDGRAMPIPE_H


#include "IPipe.h"

class BtmDGramPipe : public IPipe {
public:
    explicit BtmDGramPipe(uv_udp_t *dgram);

    int Init() override;

    int Output(ssize_t nread, const rbuf_t *buf) override;

    int Send(ssize_t nread, const rbuf_t *buf) override;

    int Close() override;

protected:
    static void send_cb(uv_udp_send_t *req, int status);
    static void recv_cb(uv_udp_t *handle, ssize_t nread, const uv_buf_t *buf, const struct sockaddr *addr,
                        unsigned flags);
private:
    using IPipe::SetOutputCb;
    using IPipe::Input;

    uv_udp_t *mDgram = nullptr;
    IUINT32 mCnt = 0;
};


#endif //RPIPE_BTMDGRAMPIPE_H
