//
// Created on 11/28/17.
//

#ifndef RPIPE_UDPBTMPIPE_H
#define RPIPE_UDPBTMPIPE_H


#include "BtmPipe.h"

class UdpBtmPipe : public BtmPipe {
public:
    UdpBtmPipe(int fd, uv_loop_t *loop);

protected:
    int outputCb(ssize_t nread, const rbuf_t *buf) override;
    int onReadable(uv_poll_t *handle) override;

};


#endif //RPIPE_UDPBTMPIPE_H
