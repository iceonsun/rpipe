//
// Created on 11/18/17.
//

#include <cassert>
#include <nmq.h>
#include "BridgePipe.h"

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
            pipe = onFreshData(nread, buf);   // todo
            if (!pipe) {
                fprintf(stderr, "null pipe. unrecognized data! ignore\n");
                return 0;   //
            }
        }
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
        fprintf(stderr, "topPipes.size: %d, line: %d\n", mTopPipes.size(), __LINE__);
        mTopPipes.erase(it);    // todo: can move this to cleanUseless
        fprintf(stderr, "topPipes.size: %d, line: %d\n", mTopPipes.size(), __LINE__);
        IPipe *pipe = it->second;
        pipe->SetOutputCb(nullptr);
        pipe->SetOnErrCb(nullptr);
//        auto key = it->first;
//
//        auto pos = mAddrMap.find(key);
//        mAddrMap.erase(pos);
//        free(pos->second); // delete addr

        mUselessPipe.push_back(it->second);
    }
    return 0;
}

IPipe *BridgePipe::FindPipe(ssize_t nread, const rbuf_t *buf) {
    KeyType key = keyForRawData(nread, buf);
    return (!key.empty()) ? mTopPipes[key] : nullptr;
}


// if (key) must be true
int BridgePipe::AddPipe(BridgePipe::KeyType key, IPipe *pipe) {
    if (!key.empty() && pipe) {
        pipe->Init();
        auto out = std::bind(&BridgePipe::Send, this, std::placeholders::_1, std::placeholders::_2);
        pipe->SetOutputCb(out);

        auto err = std::bind(&BridgePipe::OnTopPipeError, this, std::placeholders::_1, std::placeholders::_2);
        pipe->SetOnErrCb(err);

        mTopPipes.insert({key, pipe});
//        mAddrMap.insert({key, addr});
    }
}

void BridgePipe::Flush(IUINT32 curr) {
    cleanUseless();
    fprintf(stderr, "topPipes.size: %d, line: %d\n", mTopPipes.size(), __LINE__);
    for (auto &e: mTopPipes) {
        e.second->Flush(curr);
    }
    mBtmPipe->Flush(curr);  // normall this will not be necessary if btm pipe input data passively from other soruce
}

void BridgePipe::cleanUseless() {
    for (auto p: mUselessPipe) {
        p->Close();
        delete p;
    }
    mUselessPipe.clear();
}

IPipe *BridgePipe::onFreshData(ssize_t nread, const rbuf_t *buf) {
    if (mFreshDataCb) {
        return mFreshDataCb(nread, buf);
    }
    return nullptr;
}

void BridgePipe::SetOnFreshDataCb(BridgePipe::OnFreshDataCb cb) {
    mFreshDataCb = cb;
}

BridgePipe::KeyType BridgePipe::keyForRawData(ssize_t nread, const rbuf_t *data) {
    if (mHashFunc) {
        return mHashFunc(nread, data);
    }
    return nullptr;
}

void BridgePipe::OnTopPipeError(IPipe *pipe, int err) {
    fprintf(stderr, "pipe %p error: %d, %s\n", pipe, err, uv_strerror(err));
    RemovePipe(pipe);
}

void BridgePipe::OnBtmPipeError(IPipe *pipe, int err) {
    assert(pipe == mBtmPipe);
    fprintf(stderr, "btm pipe %p error: %d, %s\n", pipe, err, uv_strerror(err));
    OnError(this, err);
}

void BridgePipe::SetHashRawDataFunc(BridgePipe::HashFunc fn) {
    mHashFunc = fn;
}