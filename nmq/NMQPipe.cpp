//
// Created on 1/8/18.
//

#include <nmq/nmq.h>
#include <plog/Log.h>
#include "NMQPipe.h"

NMQPipe::NMQPipe(uint32_t conv, IPipe *topPipe) : INMQPipe(conv, topPipe) {
}

void NMQPipe::nmqRecvDone() {
    SetOnRecvCb(nullptr);
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

    // todo: when recv done, delay closing stream because it may not finish writing
    LOGV_IF(nret == NMQ_RECV_EOF) << "nmq_recv, eof " << nret;;
    LOGE_IF(nret < 0 && nret != NMQ_RECV_EOF) << "nmq_recv, error " << nret;

    if (nret == NMQ_RECV_EOF) {
        nmqRecvDone();
    }
    return nret < 0 ? nret : tot;
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
        LOGV << "TopPipe error: " << nread;
        nmq_shutdown_send(mNmq);    // todo: remove this from nmq. delay closing
//        Output(nread, buf);
    } else if (nread > 0) {
        ssize_t nleft = nread;
        while (nleft > 0) {
            int n = nmq_send(mNmq, buf->base + (nread - nleft), nleft);
            nleft -= n;
        }
    }
    return nread;
}

int32_t NMQPipe::nmqOutput(const char *data, const int len, struct nmq_s *nmq) {
    int nret = -1;
    if (len > 0) {
        rbuf_t buf = {0};
        char tbuf[len] = {0};
        memcpy(tbuf, data, len);
        buf.base = tbuf;
        buf.len = len;
        nret = Output(len, &buf);
    } else if (len == NMQ_SEND_EOF) {
        LOGV << "nmq send done";
        nmqSendDone();
        nret = 0;
    } else {
        LOGE << "unrecognized len: " << len;
    }

    return nret;
}

void NMQPipe::Flush(uint32_t curr) {
    INMQPipe::Flush(curr);
    nmqRecv(mNmq);  // necessary!! or eof segment will not be pulled of queue since they may not come in order
}

void NMQPipe::onSendFailed(NMQ *q, uint32_t sn) {
    LOGV << "send failure. close self";
    nmqSendDone();
}
