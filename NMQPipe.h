//
// Created on 11/13/17.
//

#ifndef RPIPE_NMQPIPE_H
#define RPIPE_NMQPIPE_H


#include <nmq.h>
#include "IPipe.h"

class NMQPipe : public IPipe {
public:
    NMQPipe(IUINT32 conv, IPipe *topPipe);

    int Init() override;

    int Send(ssize_t nread, const rbuf_t *buf) override;

    int Input(ssize_t nread, const rbuf_t *buf) override;

    int Close() override;

    void Flush(IUINT32 curr) override;


protected:
    static IINT32 nmqOutputCb(const char *data, const int len, struct nmq_s *nmq, void *arg);

private:
    int nmqRecv(NMQ *nmq);
private:
    IUINT32 mConv = 0;
    NMQ *mNmq = nullptr;
    IPipe *mTopPipe = nullptr;
};


#endif //RPIPE_NMQPIPE_H
