//
// Created on 1/8/18.
//

#ifndef RPIPE_STREAMNMQPIPE_H
#define RPIPE_STREAMNMQPIPE_H

#include "INMQPipe.h"

class NMQPipe : public INMQPipe {
public:
    NMQPipe(uint32_t conv, IPipe *topPipe);

    int Input(ssize_t nread, const rbuf_t *buf) override;

    int Send(ssize_t nread, const rbuf_t *buf) override;

    void Flush(uint32_t curr) override;

    int32_t nmqOutput(const char *data, const int len, struct nmq_s *nmq) override;

protected:
    void onSendFailed(struct nmq_s *q, uint32_t sn) override;

    void nmqRecvDone();

    void nmqSendDone();

private:
    int nmqRecv(struct nmq_s *nmq);
};


#endif //RPIPE_STREAMNMQPIPE_H
