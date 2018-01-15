//
// Created on 1/12/18.
//

#include <nmq/nmq.h>
#include <plog/Log.h>
#include "INMQPipe.h"

INMQPipe::INMQPipe(IUINT32 conv, IPipe *topPipe) : ITopContainerPipe(topPipe) {
    mConv = conv;
    mNmq = nmq_new(mConv, this);
}

int INMQPipe::Init() {
    ITopContainerPipe::Init();
    nmq_set_output_cb(mNmq, nmqOutputCb);
    nmq_start(mNmq);

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

void INMQPipe::Flush(IUINT32 curr) {
    ITopContainerPipe::Flush(curr);
    nmq_flush(mNmq, curr);
}

void INMQPipe::SetWndSize(IUINT32 sndwnd, IUINT32 rcvwnd) {
    nmq_set_wnd_size(mNmq, sndwnd, rcvwnd);
}

IINT32 INMQPipe::nmqOutputCb(const char *data, const int len, struct nmq_s *nmq, void *arg) {
    auto pipe = static_cast<INMQPipe *>(nmq->arg);
    return pipe->nmqOutput(data, len, nmq);
}

void INMQPipe::SetFlowControl(bool fc) {
    nmq_set_fc_on(mNmq, fc ? 1 : 0);
}

void INMQPipe::SetMSS(IUINT32 MSS) {
    nmq_set_nmq_mtu(mNmq, MSS);
}

void INMQPipe::SetInterval(IUINT16 interval) {
    nmq_set_interval(mNmq, interval);
}

void INMQPipe::SetTolerance(IUINT8 tolerance) {
    nmq_set_trouble_tolerance(mNmq, tolerance);
}

void INMQPipe::SetAckLimit(IUINT8 ackLim) {
    nmq_set_dup_acks_limit(mNmq, ackLim);
}

void INMQPipe::SetSegPoolCap(IUINT8 CAP) {
    nmq_set_segment_pool_cap(mNmq, CAP);
}

void INMQPipe::SetFcAlpha(float alpha) {
    nmq_set_fc_alpha(mNmq, alpha);
}

void INMQPipe::SetSteady(bool steady) {
    nmq_set_steady(mNmq, static_cast<IUINT8>(steady));
}
