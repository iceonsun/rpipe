//
// Created on 1/12/18.
//

#ifndef RPIPE_INMQPIPE_H
#define RPIPE_INMQPIPE_H


#include "../ITopContainerPipe.h"

struct nmq_s;

class INMQPipe : public ITopContainerPipe {
public:
    INMQPipe(IUINT32 conv, IPipe *topPipe);

    int Init() override;

    void Flush(IUINT32 curr) override;

    int Close() override;

    void SetWndSize(IUINT32 sndwnd, IUINT32 rcvwnd);

    void SetFlowControl(bool fc);

    void SetMSS(IUINT32 MSS);

    void SetInterval(IUINT16 interval);

    void SetTolerance(IUINT8 tolerance);

    void SetAckLimit(IUINT8 ackLim);

    void SetSegPoolCap(IUINT8 CAP);

    void SetFcAlpha(float alpha);

    void SetSteady(bool steady);

    virtual IINT32 nmqOutput(const char *data, const int len, struct nmq_s *nmq) = 0;

protected:
    virtual void onSendFailed(struct nmq_s *q, uint32_t sn) = 0;

private:
    static void sendFailureCb(struct nmq_s *q, uint32_t sn);

    static IINT32 nmqOutputCb(const char *data, const int len, struct nmq_s *nmq, void *arg);

protected:
    IUINT32 mConv = 0;
    struct nmq_s *mNmq = nullptr;
};


#endif //RPIPE_INMQPIPE_H
