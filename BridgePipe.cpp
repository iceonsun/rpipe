//
// Created on 11/18/17.
//

#include <cassert>
#include <nmq.h>
#include <syslog.h>
#include "BridgePipe.h"
#include "debug.h"
#include <utility>
BridgePipe::BridgePipe(IPipe *btmPipe) {
    mBtmPipe = btmPipe;
}

int BridgePipe::Init() {
    mBtmPipe->Init();   // todo init must be called. make IPipe::Init non pure virtual. adn create start.

    auto out = std::bind(&IPipe::Send, mBtmPipe, std::placeholders::_1, std::placeholders::_2);
    SetOutputCb(out);

    auto rcv = std::bind(&BridgePipe::Input, this, std::placeholders::_1, std::placeholders::_2);
    mBtmPipe->SetOnRecvCb(rcv);

    auto err = std::bind(&BridgePipe::OnBtmPipeError, this, std::placeholders::_1, std::placeholders::_2);
    mBtmPipe->SetOnErrCb(err);
    return 0;
}

int BridgePipe::Close() {
    if (!mTopPipes.empty()) {
        removeAll();
    }

    if (mBtmPipe) {
        mBtmPipe->Close();
        SetOutputCb(nullptr);
        mBtmPipe->SetOnRecvCb(nullptr);
        mBtmPipe->SetOnErrCb(nullptr);
        delete mBtmPipe;
        mBtmPipe = nullptr;
    }

    if (!mUselessPipe.empty()) {
        cleanUseless();
    }
    return 0;
}

int BridgePipe::Input(ssize_t nread, const rbuf_t *buf) {
    int nret = nread;
    IPipe *pipe = nullptr;
    if (nread > 0 && buf) {
        pipe = FindPipe(nread, buf);
        // cannot be null
        if (!pipe) {
//            pipe = onFreshData(nread, buf);   // todo
            auto pipe2 = onFreshData(nread, buf);   // todo
            pipe = FindPipe(nread, buf);
            printf("pipe1: %p, pipe2: %p", pipe, pipe2);
            if (!pipe) {
                debug(LOG_ERR, "null pipe. unrecognized data! ignore\n");
                return 0;   //
            }
        }
        debug(LOG_INFO, "pipe: %p", pipe);
        nret = pipe->Input(nread, buf);
        if (nret < 0) {
            OnTopPipeError(pipe, nret);
        }
    } else if (nret < 0) {
        OnBtmPipeError(mBtmPipe, nret);
    } else {
        fprintf(stderr, "invalid data length, nread: %d\n", nret);
        OnBtmPipeError(mBtmPipe, 0);
    }

    return nret;
}


int BridgePipe::RemovePipe(IPipe *pipe) {
    for (auto it = mTopPipes.begin(); it != mTopPipes.end(); it++) {
        if (it->second == pipe) {
            return doRemove(it);
        }
    }
    fprintf(stderr, "failed to find and remove pipe: %p\n", pipe);
    return 0;
}

int BridgePipe::removeAll() {
    for (auto it = mTopPipes.begin(); it != mTopPipes.end(); it++) {
        doRemove(it);
    }
    return 0;
}

int BridgePipe::doRemove(std::map<KeyType, IPipe *>::iterator it) {
    if (it != mTopPipes.end()) {
        mTopPipes.erase(it);
        IPipe *pipe = it->second;
        pipe->SetOutputCb(nullptr);
        pipe->SetOnErrCb(nullptr);

        mUselessPipe.push_back(it->second);
    }
    return 0;
}

IPipe *BridgePipe::FindPipe(ssize_t nread, const rbuf_t *buf) {
    KeyType key = keyForRawData(nread, buf);
//    return (!key.empty()) ? mTopPipes[key] : nullptr;  // bug!!!!
    if (!key.empty()) {
        auto it = mTopPipes.find(key);
        return it != mTopPipes.end()? it->second: nullptr;
    }
    return nullptr;
}


// if (key) must be true
int BridgePipe::AddPipe(BridgePipe::KeyType key, IPipe *pipe) {
    if (!key.empty() && pipe) {
        pipe->Init();
        auto out = std::bind(&BridgePipe::PSend, this, pipe, std::placeholders::_1, std::placeholders::_2);
        pipe->SetOutputCb(out);

        auto err = std::bind(&BridgePipe::OnTopPipeError, this, std::placeholders::_1, std::placeholders::_2);
        pipe->SetOnErrCb(err);

        auto ret = mTopPipes.insert(std::make_pair(key, pipe));
        debug(LOG_INFO, "key: %s, pipe1: %p, pipe2: %p, ok:%d", key.c_str(), pipe, mTopPipes[key], ret.second);
    }
}

void BridgePipe::Flush(IUINT32 curr) {
    cleanUseless();
    mBtmPipe->Flush(curr);  // normall this will not be necessary if btm pipe input data passively from other soruce

    for (auto &e: mTopPipes) {
        e.second->Flush(curr);
    }
}

void BridgePipe::cleanUseless() {
    for (auto p: mUselessPipe) {
        p->Close();
        delete p;
    }
    mUselessPipe.clear();
}

IPipe *BridgePipe::onFreshData(ssize_t nread, const rbuf_t *buf) {
    assert(mFreshDataCb != nullptr);
    return mFreshDataCb(nread, buf);
}

void BridgePipe::SetOnFreshDataCb(BridgePipe::OnFreshDataCb cb) {
    mFreshDataCb = cb;
}

BridgePipe::KeyType BridgePipe::keyForRawData(ssize_t nread, const rbuf_t *data) {
    assert(mHashFunc != nullptr);
    return mHashFunc(nread, data);
}

void BridgePipe::OnTopPipeError(IPipe *pipe, int err) {
    fprintf(stderr, "pipe %p error: %d, %s\n", pipe, err, uv_strerror(err));
    RemovePipe(pipe);
}

void BridgePipe::OnBtmPipeError(IPipe *pipe, int err) {
    assert(pipe == mBtmPipe);   // typically sendto will not fail
    fprintf(stderr, "btm pipe %p error: %d, %s\n", pipe, err, uv_strerror(err));
    OnError(this, err);
}

void BridgePipe::SetHashRawDataFunc(BridgePipe::HashFunc fn) {
    mHashFunc = fn;
}

int BridgePipe::PSend(IPipe *pipe, ssize_t nread, const rbuf_t *buf) {
    if (nread < 0) {    // error occurs. eof or other
        OnTopPipeError(pipe, nread);
        return nread;
    } else if (nread == 0) {    // do nothing
        return 0;
    } else {
        int n = Send(nread, buf);
        if (n < 0) {
            debug(LOG_ERR, "failed to send data. nread: %d, pipe: %p\n", nread, pipe);
            OnTopPipeError(pipe, n);    // just close this pipe
        }
        return n;
    }
}

