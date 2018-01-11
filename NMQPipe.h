//
// Created on 1/8/18.
//

#ifndef RPIPE_STREAMNMQPIPE_H
#define RPIPE_STREAMNMQPIPE_H

#include "ITopContainerPipe.h"

struct nmq_s;

class NMQPipe : public ITopContainerPipe {
public:
    NMQPipe(IUINT32 conv, IPipe *topPipe);

    int Init() override;

    void Flush(IUINT32 curr) override;

    int Close() override;

    int Input(ssize_t nread, const rbuf_t *buf) override;

    int Send(ssize_t nread, const rbuf_t *buf) override;

protected:
    static IINT32 nmqOutputCb(const char *data, const int len, struct nmq_s *nmq, void *arg);
    void nmqRecvDone();
    void nmqSendDone();

private:
    int nmqRecv(struct nmq_s *nmq);

private:
    IUINT32 mConv = 0;

    struct nmq_s *mNmq = nullptr;
};


#endif //RPIPE_STREAMNMQPIPE_H
