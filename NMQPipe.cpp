//
// Created on 11/13/17.
//

#include <cassert>
#include "NMQPipe.h"

NMQPipe::NMQPipe(IUINT32 conv, IPipe *topPipe) : ITopContainerPipe(topPipe) {
    mConv = conv;
}

NMQPipe::~NMQPipe() {
    debug(LOG_ERR, "");
}

int NMQPipe::Init() {
    ITopContainerPipe::Init();

    mNmq = nmq_new(mConv, this);
    nmq_set_output_cb(mNmq, nmqOutputCb);
    nmq_start(mNmq);

    debug(LOG_INFO, "");

    return 0;
}

int NMQPipe::Close() {
    ITopContainerPipe::Close();
    if (mNmq) {
        debug(LOG_ERR, "mSndTot: %d, mRcvTot: %d", mSndTot, mRcvTot);
        nmq_destroy(mNmq);
        mNmq = nullptr;
    }
    return 0;
}

// > 0 for number of bytes sent
// = 0 no bytes sent
// < 0 error occurs.
int NMQPipe::Send(ssize_t nread, const rbuf_t *buf) {
    int nret = nread;
    if (nread > 0) {
        nret = nmq_send(mNmq, buf->base, nread);
        if (nret > 0) {
            mSndTot += nret;
        }
        nmq_flush(mNmq, iclock());  // todo: check if necessary
        debug(LOG_ERR, "nmq_send %d bytes. ret: %d. curr: %d", nread, nret, iclock() % 10000);
    } else if (nread < 0) {
        debug(LOG_ERR, "err state: %d", nread);
        BlockTop();
        mErrState = nread;
        nmq_shutdown_send(mNmq, nmqShutdownCb);
//        Output(nread, buf); // this means report error: nread
    }
    return nret;
}

int NMQPipe::Input(ssize_t nread, const rbuf_t *buf) {
    assert(nread > 0);  // it's caller's responsibility to check nread

    int nret = nread;
    if (nread > 0) {
        // omit allocating buf here. becuase we know nmq will not reuse buf.
        nret = nmq_input(mNmq, buf->base, nread);
        if (NMQ_ERR_CONV_DIFF == nret) {
            debug(LOG_ERR, "self conv: %u, data conv: %u", mConv, nmq_get_conv(buf->base));
            return NMQ_ERR_CONV_DIFF;
        }   // other error just ignore it
//        nmq_flush(mNmq, iclock()); // todo: check if necessary or better place
        debug(LOG_INFO, "nmq_input %d bytes. ret: %d, current: %d", nread, nret, iclock() % 10000);
        if (nret >= 0) {
            int n = nmqRecv(mNmq);
            return n;
        }
    }
    return nret;
}

IINT32 NMQPipe::nmqOutputCb(const char *data, const int len, struct nmq_s *nmq, void *arg) {
    int nret = -1;
    auto pipe = static_cast<NMQPipe *>(nmq->arg);
    if (len > 0) {
        rbuf_t buf = {0};
        buf.base = const_cast<char *>(data);   // caution!! reuse ptr
        buf.len = len;
        debug(LOG_INFO, "nmqOutputCb, %d bytes. curr: %d", len, iclock() % 10000);
        nret = pipe->Output(len, &buf);
    } else if (len == NMQ_EOF) {
        pipe->nmqRecvDone();
        nret = 0;
    } else {
        debug(LOG_ERR, "unrecognized len: %d", len);
    }

    return nret;
}

int NMQPipe::nmqRecv(NMQ *nmq) {
    int nret = 0;
    int tot = 0;

    rbuf_t buf = {0};
    const int SIZE = 65536;
    char base[SIZE] = {0};       // todo: change to pipe_bufsiz
    buf.base = base;
    buf.len = SIZE;

    int n = 0;
    // todo: 是bufsize太小
    while ((nret = nmq_recv(nmq, buf.base, SIZE)) > 0) {
        debug(LOG_ERR, "nret: %d", nret);
        mRcvTot += nret;
        tot += nret;
        n = OnRecv(nret, &buf);
        if (n < 0) {
            break;
        }
    }
    if (nret < 0) {
        debug(LOG_ERR, "nmq_recv error: %d", nret);
    }
    return n < 0 ? n : tot;
}

void NMQPipe::Flush(IUINT32 curr) {
    ITopContainerPipe::Flush(curr);
    nmq_flush(mNmq, curr);
//    nmq_update(mNmq, curr);
}

void NMQPipe::nmqShutdownCb(NMQ *q) {
    NMQPipe *pipe = static_cast<NMQPipe *>(q->arg);
    pipe->nmqSendDone();
}

void NMQPipe::nmqSendDone() {
    debug(LOG_ERR, "nmq send done.");
    rbuf_t rbuf;
    Output(mErrState, &rbuf);
}

void NMQPipe::nmqRecvDone() {
    debug(LOG_ERR, "nmq recv done.");
    rbuf_t rbuf;
    Output(UV_EOF, &rbuf);
}
