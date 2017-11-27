//
// Created on 11/18/17.
//

#include <cassert>
#include <nmq.h>
#include <syslog.h>
#include "BridgePipe.h"
#include "debug.h"

BridgePipe::BridgePipe(IPipe *btmPipe) {
    mBtmPipe = btmPipe;
}

int BridgePipe::Init() {
    IPipe::Init();

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
    if (nread >= SessionPipe::HEAD_LEN) {
        SessionPipe::KeyType key = SessionPipe::BuildKey(nread, buf);
        SessionPipe *sess = FindPipe(key);
        debug(LOG_ERR, "key: %s", key.c_str());
        if (SessionPipe::IsCloseSignal(nread, buf)) {
            if (!sess) {    // if doesn't exist. we do nothing. otherwise we pass it to sessppe.
                return 0;
            }
        } else {
            if (!sess) {
                sess = onCreateNewPipe(key, buf->data);
                if (!sess) {
                    return 0;   // dont' create pipe
                }
            }
        }
        nret = sess->Input(nread, buf);
        if (nret < 0) {
            debug(LOG_ERR, "session pipe error: %d.  close pipe: %p", nret, sess);
            RemovePipe(sess);   // todo: add onpipeclose
        }
    } else if (nread < 0) {
        OnBtmPipeError(mBtmPipe, nread);
    } else {
        debug(LOG_ERR, "invalid data, nread: %d\n", nread);
    }
    return nret;
}

int BridgePipe::RemovePipe(SessionPipe *pipe) {
    for (auto it = mTopPipes.begin(); it != mTopPipes.end(); it++) {
        if (it->second == pipe) {
            return doRemove(it);
        }
    }
    debug(LOG_ERR, "failed to find and remove pipe: %p\n", pipe);
    return 0;
}

int BridgePipe::removeAll() {
    for (auto it = mTopPipes.begin(); it != mTopPipes.end(); it++) {
        doRemove(it);
    }
    return 0;
}

int BridgePipe::doRemove(std::map<SessionPipe::KeyType, SessionPipe *>::iterator it) {
    if (it != mTopPipes.end()) {
        mTopPipes.erase(it);
        IPipe *pipe = it->second;
        pipe->SetOutputCb(nullptr);
        pipe->SetOnErrCb(nullptr);

        mUselessPipe.push_back(it->second);
    }
    return 0;
}

SessionPipe *BridgePipe::FindPipe(const SessionPipe::KeyType &key) const {
    auto it = mTopPipes.find(key);
    return it != mTopPipes.end() ? it->second : nullptr;
}

void BridgePipe::Start() {
    IPipe::Start();
    mBtmPipe->Start();
}

// if (key) must be true
int BridgePipe::AddPipe(SessionPipe *pipe) {
    if (pipe && !pipe->GetKey().empty()) {
        const SessionPipe::KeyType &key = pipe->GetKey();

        auto ret = mTopPipes.insert(std::make_pair(key, pipe));
        debug(LOG_ERR, "key: %s, pipe1: %p, pipe2: %p, ok:%d", key.c_str(), pipe, mTopPipes[key], ret.second);
        if (!ret.second) {
            debug(LOG_ERR, "insert failed. duplicate pipe");
            return -1;
        }

        pipe->Init();
        pipe->Start();
        auto out = std::bind(&BridgePipe::PSend, this, pipe, std::placeholders::_1, std::placeholders::_2);
        pipe->SetOutputCb(out);

        // no need cast. caputure parameter pipe
        pipe->SetOnErrCb([this](IPipe *p, int err) {
            SessionPipe *sess = dynamic_cast<SessionPipe *>(p);

#ifndef NNDEBUG
            this->OnTopPipeError(sess, err);
#else
            if (sess) {
                this->OnTopPipeError(sess, err);
            } else {
                debug(LOG_ERR, "dynamic_cast failed");
            }
#endif
        });
    }
    return 0;
}

void BridgePipe::Flush(IUINT32 curr) {
    mBtmPipe->Flush(curr);  // normall this will not be necessary if btm pipe input data passively from other soruce

    for (auto &e: mTopPipes) {
        e.second->Flush(curr);
    }
    cleanUseless();
}

void BridgePipe::cleanUseless() {
    for (auto p: mUselessPipe) {
        p->Close();
        delete p;
    }
    mUselessPipe.clear();
}


void BridgePipe::OnTopPipeError(SessionPipe *pipe, int err) {
    debug(LOG_ERR, "pipe %p error: %d, %s\n", pipe, err, uv_strerror(err));
    RemovePipe(pipe);
}

void BridgePipe::OnBtmPipeError(IPipe *pipe, int err) {
    assert(pipe == mBtmPipe);   // typically sendto will not fail
    debug(LOG_ERR, "btm pipe %p error: %d, %s\n", pipe, err, uv_strerror(err));
    OnError(this, err);
}

int BridgePipe::PSend(SessionPipe *pipe, ssize_t nread, const rbuf_t *buf) {
    if (nread > 0) {
        return Send(nread, buf);
    } else if (nread < 0) {    // error occurs. eof or other
        OnTopPipeError(pipe, nread);
        return nread;
    }

    return 0;
}

SessionPipe *BridgePipe::onCreateNewPipe(const SessionPipe::KeyType &key, void *addr) {
    if (mCreateNewPipeCb) {
        return mCreateNewPipeCb(key, addr);
    }
    return nullptr;
}

void BridgePipe::SetOnCreateNewPipeCb(const OnCreatePipeCb &cb) {
    mCreateNewPipeCb = cb;
}

