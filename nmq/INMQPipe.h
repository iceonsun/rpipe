//
// Created on 1/12/18.
//

#ifndef RPIPE_INMQPIPE_H
#define RPIPE_INMQPIPE_H


#include "../ITopContainerPipe.h"

struct nmq_s;

class INMQPipe : public ITopContainerPipe {
public:
    INMQPipe(uint32_t conv, IPipe *topPipe);

    int Init() override;

    void Flush(uint32_t curr) override;

    int Close() override;

    void SetWndSize(uint32_t sndwnd, uint32_t rcvwnd);

    void SetFlowControl(bool fc);

    void SetNmqMTU(uint32_t mtu);

    void SetInterval(uint16_t interval);

    void SetTolerance(uint8_t tolerance);

    void SetAckLimit(uint8_t ackLim);

    void SetSegPoolCap(uint8_t CAP);

    void SetFcAlpha(float alpha);

    void SetSteady(bool steady);

    virtual int32_t nmqOutput(const char *data, const int len, struct nmq_s *nmq) = 0;

protected:
    virtual void onSendFailed(struct nmq_s *q, uint32_t sn) = 0;

private:
    static void sendFailureCb(struct nmq_s *q, uint32_t sn);

    static int32_t nmqOutputCb(const char *data, const int len, struct nmq_s *nmq, void *arg);

protected:
    uint32_t mConv = 0;
    struct nmq_s *mNmq = nullptr;
};


#endif //RPIPE_INMQPIPE_H
