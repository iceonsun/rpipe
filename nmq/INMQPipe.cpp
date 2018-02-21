//
// Created on 1/12/18.
//

#include <nmq/nmq.h>
#include <plog/Log.h>
#include "INMQPipe.h"

INMQPipe::INMQPipe(uint32_t conv, IPipe *topPipe) : ITopContainerPipe(topPipe) {
    mConv = conv;
    mNmq = nmq_new(mConv, this);
}

int INMQPipe::Init() {
    ITopContainerPipe::Init();
    nmq_set_output_cb(mNmq, nmqOutputCb);
    nmq_start(mNmq);
    nmq_set_max_attempt(mNmq, 20, sendFailureCb);
    return 0;
}

int INMQPipe::Close() {
    ITopContainerPipe::Close();

    if (mNmq) {
        nmq_stat_t *st = &mNmq->stat;
        LOGD << "fc: " << (int)mNmq->fc_on << ", sndwnd: " << mNmq->MAX_SND_BUF_NUM << ", rcvwnd: " << mNmq->MAX_RCV_BUF_NUM << ", interval: " << mNmq->flush_interval << ", bytes_per_flush: " << mNmq->BYTES_PER_FLUSH;
        LOGD << "nmq_stat, rtt: " << st->nrtt_tot * 1.0 / st->nrtt << "ms, oversend ratio: "
             << st->bytes_send_tot * 1.0 / st->bytes_send;
        nmq_destroy(mNmq);
        mNmq = nullptr;
    }
    return 0;
}

void INMQPipe::Flush(uint32_t curr) {
    ITopContainerPipe::Flush(curr);
    nmq_flush(mNmq, curr);
}

void INMQPipe::SetWndSize(uint32_t sndwnd, uint32_t rcvwnd) {
    nmq_set_wnd_size(mNmq, sndwnd, rcvwnd);
}

int32_t INMQPipe::nmqOutputCb(const char *data, const int len, struct nmq_s *nmq, void *arg) {
    auto pipe = static_cast<INMQPipe *>(nmq->arg);
    return pipe->nmqOutput(data, len, nmq);
}

void INMQPipe::SetFlowControl(bool fc) {
    nmq_set_fc_on(mNmq, fc ? 1 : 0);
}

void INMQPipe::SetNmqMTU(uint32_t mtu) {
    nmq_set_nmq_mtu(mNmq, mtu);
}

void INMQPipe::SetInterval(uint16_t interval) {
    nmq_set_interval(mNmq, interval);
}

void INMQPipe::SetTolerance(uint8_t tolerance) {
    nmq_set_trouble_tolerance(mNmq, tolerance);
}

void INMQPipe::SetAckLimit(uint8_t ackLim) {
    nmq_set_dup_acks_limit(mNmq, ackLim);
}

void INMQPipe::SetSegPoolCap(uint8_t CAP) {
    nmq_set_segment_pool_cap(mNmq, CAP);
}

void INMQPipe::SetFcAlpha(float alpha) {
    nmq_set_fc_alpha(mNmq, alpha);
}

void INMQPipe::SetSteady(bool steady) {
    nmq_set_steady(mNmq, static_cast<uint8_t>(steady));
}

void INMQPipe::sendFailureCb(NMQ *q, uint32_t sn) {
    INMQPipe *pipe1 = static_cast<INMQPipe *>(q->arg);
    pipe1->onSendFailed(q, sn);
}
