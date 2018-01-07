//
// Created on 11/13/17.
//

#ifndef RPIPE_NMQPIPE_H
#define RPIPE_NMQPIPE_H


#include <nmq.h>
#include <syslog.h>
#include "thirdparty/debug.h"
#include "ITopContainerPipe.h"
#include "IRdWriter.h"

class NMQPipe : public IPipe {
public:

    NMQPipe(IUINT32 conv, IRdWriter *rdwr);

    ~NMQPipe() override;

    int Init() override;

    int Input(ssize_t nread, const rbuf_t *buf) override;

    int Close() override;

    void Flush(IUINT32 curr) override;

protected:
    static IINT32 nmqOutputCb(const char *data, const int len, struct nmq_s *nmq, void *arg);
    static IINT32 read_cb(NMQ *nmq, char *buf, int len, int *err);

    void nmqRecvDone();
    void nmqSendDone();
    int onRecvCb(ssize_t nread, const rbuf_t *buf);

private:
    int nmqRecv(NMQ *nmq);

private:
    IUINT32 mConv = 0;
    NMQ *mNmq = nullptr;

    IRdWriter *mRdWriter;
};


#endif //RPIPE_NMQPIPE_H
