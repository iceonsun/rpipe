//
// Created on 11/20/17.
//

#ifndef RPIPE_RAWPIPE_H
#define RPIPE_RAWPIPE_H


#include "../../IPipe.h"
#include "../../debug.h"

class RawPipe : public IPipe {
public:
    virtual ~RawPipe() {debug(LOG_ERR, "delete");};
    int Init() override;

    int OnRecv(ssize_t nread, const rbuf_t *buf) override;

    int Close() override;

};


#endif //RPIPE_RAWPIPE_H
