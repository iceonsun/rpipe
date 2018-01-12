//
// Created on 1/8/18.
//

#ifndef RPIPE_STREAMNMQPIPE_H
#define RPIPE_STREAMNMQPIPE_H

#include "INMQPipe.h"

class NMQPipe : public INMQPipe {
public:
    NMQPipe(IUINT32 conv, IPipe *topPipe);

    int Input(ssize_t nread, const rbuf_t *buf) override;

    int Send(ssize_t nread, const rbuf_t *buf) override;

    void Flush(IUINT32 curr) override;

    IINT32 nmqOutput(const char *data, const int len, struct nmq_s *nmq) override;

protected:

    void nmqRecvDone();

    void nmqSendDone();

private:
    int nmqRecv(struct nmq_s *nmq);
};


#endif //RPIPE_STREAMNMQPIPE_H
