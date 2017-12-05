//
// Created on 11/13/17.
//

#include <cassert>
#include "NMQPipe.h"

NMQPipe::NMQPipe(IUINT32 conv, IRdWriter *rdwr) {
    mConv = conv;
    mRdWriter = rdwr;
    IPipe *pipe = this;
    mRdWriter->SetOnErrCb([pipe](int err) {
        debug(LOG_ERR, "rdwriter err: %d", err);
        pipe->OnError(pipe, err);
    });
    auto fn = std::bind(&NMQPipe::onRecvCb, this, std::placeholders::_1, std::placeholders::_2);
    SetOnRecvCb(fn);
}

NMQPipe::~NMQPipe() {
    debug(LOG_ERR, "");
}

int NMQPipe::Init() {
    IPipe::Init();

    mNmq = nmq_new(mConv, this);
    nmq_set_read_cb(mNmq, read_cb);
    nmq_set_output_cb(mNmq, nmqOutputCb);
    nmq_start(mNmq);

    return 0;
}

int NMQPipe::Close() {
    if (mRdWriter) {
        mRdWriter->Close();
        delete mRdWriter;
        mRdWriter = nullptr;
    }

    if (mNmq) {
        nmq_destroy(mNmq);
        mNmq = nullptr;
    }
    return 0;
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
        debug(LOG_ERR, "nmq_input %d bytes. ret: %d, current: %d", nread, nret, iclock() % 10000);
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
        debug(LOG_ERR, "nmqOutputCb, %d bytes. curr: %d", len, iclock() % 10000);
        nret = pipe->Output(len, &buf);
    } else if (len == NMQ_SEND_EOF) {
        pipe->nmqSendDone();
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

    // todo: 是bufsize太小
    while ((nret = nmq_recv(nmq, buf.base, SIZE)) > 0) {
        debug(LOG_ERR, "nmq_recv: %d", nret);
        tot += nret;
        OnRecv(nret, &buf);
    }
    if (nret < 0) {
        debug(LOG_ERR, "nmq_recv error: %d", nret);
    }
    if (nret == NMQ_RECV_EOF) {
        nmqRecvDone();
        SetOnRecvCb(nullptr);
    }
    return nret < 0 ? nret : tot;
}

void NMQPipe::Flush(IUINT32 curr) {
    IPipe::Flush(curr);
    nmq_flush(mNmq, curr);
    nmqRecv(mNmq);
}

void NMQPipe::nmqRecvDone() {
    debug(LOG_ERR, "nmq recv done.");
//    nmqRecv(mNmq);  // stack overflows
    rbuf_t rbuf;
    Output(UV_EOF, &rbuf);
}

void NMQPipe::nmqSendDone() {
//    nmqRecv(mNmq);  // no need to recv because self closed
    debug(LOG_ERR, "nmq send done.");
    rbuf_t rbuf;
    Output(UV_EOF, &rbuf);
}

IINT32 NMQPipe::read_cb(NMQ *nmq, char *buf, int len, int *err) {
    auto *pipe = static_cast<NMQPipe *>(nmq->arg);
    return pipe->mRdWriter->Read(buf, len, err);
}

int NMQPipe::onRecvCb(ssize_t nread, const rbuf_t *buf) {
    int err;
    int n = mRdWriter->Write(buf->base, nread, &err);
    return n;
}
