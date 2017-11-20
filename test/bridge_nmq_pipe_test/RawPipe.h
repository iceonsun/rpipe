//
// Created on 11/20/17.
//

#ifndef RPIPE_RAWPIPE_H
#define RPIPE_RAWPIPE_H


#include "../../IPipe.h"

class RawPipe : public IPipe {
public:
    int Init() override;

    int OnRecv(ssize_t nread, const rbuf_t *buf) override;

    int Close() override;
};


#endif //RPIPE_RAWPIPE_H
