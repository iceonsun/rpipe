//
// Created on 11/13/17.
//

#include <cassert>
#include "NMQPipe.h"

NMQPipe::NMQPipe(IUINT32 conv, IPipe *topPipe) {
    mConv = conv;
    mTopPipe = topPipe;
}

int NMQPipe::Init() {
    mTopPipe->Init();

    mNmq = nmq_new(mConv, this);
    nmq_set_output_cb(mNmq, nmqOutputCb);

    nmq_start(mNmq);

    PipeCb outcb = std::bind(&NMQPipe::Send, this, std::placeholders::_1, std::placeholders::_2);
    mTopPipe->SetOutputCb(outcb);

    PipeCb rcvcb = std::bind(&IPipe::Input, mTopPipe, std::placeholders::_1, std::placeholders::_2);
    SetOnRecvCb(rcvcb);

    mTopPipe->SetOnErrCb([this](IPipe *pipe, int err) {
        fprintf(stderr, "topPipe %p error: %d\n", pipe, err);
        this->OnError(this, err);   // replace pipe. pass error to btm
    });

    return 0;
}


int NMQPipe::Close() {
    if (mNmq) {
        nmq_destroy(mNmq);
        mNmq = nullptr;
    }
    if (mTopPipe) {
        mTopPipe->Close();
        mTopPipe->SetOutputCb(nullptr);
        SetOnRecvCb(nullptr);
        mTopPipe->SetOnErrCb(nullptr);
        delete mTopPipe;
        mTopPipe = nullptr;
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
    } else if (nread < 0) {
        nmq_update(mNmq, iclock()); // sending remaining data
        Output(nread, buf); // this means report error: nread
    }
    return nret;
}

int NMQPipe::Input(ssize_t nread, const rbuf_t *buf) {
    assert(nread > 0);  // it's caller's responsibility to check nread

    int nret = nread;
    if (nread > 0) {
        // omit allocating buf here. becuase we know nmq will not reuse buf.
        nret = nmq_input(mNmq, buf->base, nread);
        if (nret >= 0) {
            int n = nmqRecv(mNmq);
            if (n < 0) {
                return n;   // return error to caller
            }
        }
    }
    return nret;
}

IINT32 NMQPipe::nmqOutputCb(const char *data, const int len, struct nmq_s *nmq, void *arg) {
    assert(len > 0);

    int nret = -1;
    if (len > 0) {
        auto pipe = static_cast<NMQPipe *>(nmq->arg);
        rbuf_t buf = {0};
        buf.base = const_cast<char *>(data);   // caution!! reuse ptr
        buf.len = len;
        buf.data = pipe->GetTargetAddr();
        debug(LOG_INFO, "data: %p\n", data);
        nret = pipe->Output(len, &buf);
    }

    return nret;
}

int NMQPipe::nmqRecv(NMQ *nmq) {
    int nret = 0;
    int tot = 0;

    rbuf_t buf = {0};
    buf.base =(char*) malloc(PIPE_BUFSIZ);
    buf.len = PIPE_BUFSIZ;

    int n = 0;
    while ((nret = nmq_recv(nmq, buf.base, PIPE_BUFSIZ)) > 0) {
        tot += nret;
        n = OnRecv(nret, &buf);
        if (n < 0) {
            break;
        }
    }
    free(buf.base);
    return n < 0 ? n : tot;
}

void NMQPipe::Flush(IUINT32 curr) {
    nmq_update(mNmq, curr);
    mTopPipe->Flush(curr);
}