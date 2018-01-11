//
// Created on 1/8/18.
//

#include <nmq.h>
#include <plog/Log.h>
#include "NMQPipe.h"

NMQPipe::NMQPipe(IUINT32 conv, IPipe *topPipe) : ITopContainerPipe(topPipe) {
    mConv = conv;
}

int NMQPipe::Init() {
    ITopContainerPipe::Init();
    mNmq = nmq_new(mConv, this);
//    nmq_set_read_cb(mNmq, read_cb);
    nmq_set_output_cb(mNmq, nmqOutputCb);
    nmq_start(mNmq);

    return 0;
}

int NMQPipe::Close() {
    ITopContainerPipe::Close();

    if (mNmq) {
        nmq_destroy(mNmq);
        mNmq = nullptr;
    }
    return 0;
}

IINT32 NMQPipe::nmqOutputCb(const char *data, const int len, struct nmq_s *nmq, void *arg) {
    int nret = -1;
    auto pipe = static_cast<NMQPipe *>(nmq->arg);
    if (len > 0) {
        rbuf_t buf = {0};
        char tbuf[len] = {0};
        memcpy(tbuf, data, len);
        buf.base = tbuf;
        buf.len = len;
        nret = pipe->Output(len, &buf);
    } else if (len == NMQ_SEND_EOF) {
        pipe->nmqSendDone();
        nret = 0;
    } else {
        LOGE << "unrecognized len: " << len;
    }

    return nret;
}

void NMQPipe::nmqRecvDone() {
//    nmqRecv(mNmq);  // stack overflows
    rbuf_t rbuf = {0};
    Output(UV_EOF, &rbuf);
}

void NMQPipe::nmqSendDone() {
//    nmqRecv(mNmq);  // no need to recv because self closed
    rbuf_t rbuf = {0};
    Output(UV_EOF, &rbuf);
}

int NMQPipe::nmqRecv(struct nmq_s *nmq) {
    int nret = 0;
    int tot = 0;

    rbuf_t buf = {0};
    const int SIZE = 65536;
    char base[SIZE] = {0};       // todo: change to pipe_bufsiz
    buf.base = base;
    buf.len = SIZE;

    // todo: 是bufsize太小
    while ((nret = nmq_recv(nmq, buf.base, SIZE)) > 0) {
        LOGV << "nmq_recv: " << nret;
        tot += nret;
        OnRecv(nret, &buf);
    }
    if (nret < 0) {
        LOGE << "nmq_recv, error " << nret;
    }
    if (nret == NMQ_RECV_EOF) {
        nmqRecvDone();
        SetOnRecvCb(nullptr);
    }
    return nret < 0 ? nret : tot;
}

void NMQPipe::Flush(IUINT32 curr) {
    ITopContainerPipe::Flush(curr);
    nmq_flush(mNmq, curr);
    nmqRecv(mNmq);
}

int NMQPipe::Input(ssize_t nread, const rbuf_t *buf) {
    assert(nread > 0);  // it's caller's responsibility to check nread

    int nret = nread;
    if (nread > 0) {
        // omit allocating buf here. becuase we know nmq will not reuse buf.
        nret = nmq_input(mNmq, buf->base, nread);
        if (NMQ_ERR_CONV_DIFF == nret) {
            LOGE << "self conv: " << mConv << ", data conv: " << nmq_get_conv(buf->base) << " don't match";
            return NMQ_ERR_CONV_DIFF;
        }   // other error just ignore it
        LOGV << "nmq_input " << nread << " bytes, ret: " << nret;
        if (nret >= 0) {
            int n = nmqRecv(mNmq);
            return n;
        }
    }
    return nret;
}

int NMQPipe::Send(ssize_t nread, const rbuf_t *buf) {
    if (nread < 0) {
        Output(nread, buf);
    } else if (nread > 0) {
        ssize_t nleft = nread;
        while (nleft > 0) {
            int n = nmq_send(mNmq, buf->base + (nread - nleft), nleft);
            nleft -= n;
        }
    }
    return nread;

}
