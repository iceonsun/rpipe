//
// Created on 11/18/17.
//

#include <cassert>
#include <nmq/nmq.h>
#include <plog/Log.h>
#include "BridgePipe.h"

BridgePipe::BridgePipe(IPipe *btmPipe) {
    mBtmPipe = btmPipe;
}

int BridgePipe::Init() {
    IPipe::Init();

    mBtmPipe->Init();

    auto out = std::bind(&IPipe::Send, mBtmPipe, std::placeholders::_1, std::placeholders::_2);
    SetOutputCb(out);

    auto rcv = std::bind(&BridgePipe::Input, this, std::placeholders::_1, std::placeholders::_2);
    mBtmPipe->SetOnRecvCb(rcv);

    auto err = std::bind(&BridgePipe::OnBtmPipeError, this, std::placeholders::_1, std::placeholders::_2);
    mBtmPipe->SetOnErrCb(err);
    return 0;
}

int BridgePipe::Close() {
    IPipe::Close();

    if (!mTopPipes.empty()) {
        for (auto &e: mTopPipes) {
            RemovePipe(e.second);
        }
        mTopPipes.clear();
    }

    if (mBtmPipe) {
        mBtmPipe->Close();
        SetOutputCb(nullptr);
        delete mBtmPipe;
        mBtmPipe = nullptr;
    }

    if (!mErrPipes.empty()) {
        cleanErrPipes();
    }
    return 0;
}

int BridgePipe::Input(ssize_t nread, const rbuf_t *buf) {
    int nret = nread;
    if (nread >= ISessionPipe::HEAD_LEN) {
        ISessionPipe::KeyType key = ISessionPipe::BuildKey(nread, buf);
        ISessionPipe *sess = FindPipe(key);
        if (ISessionPipe::IsCloseSignal(nread, buf)) {
            if (!sess) {    // if doesn't exist. we do nothing. otherwise we pass it to sessppe.
                return 0;
            }
        } else {
            if (!sess) {
                // for client. unknown pipe should cause rst to peer.
                // for server. unknown pipe should cause new pipe created and persisted.
                sess = onCreateNewPipe(key, buf->data);
                if (!sess) {
                    return 0;   // dont' create pipe
                }
                AddPipe(sess);
            }
        }
        nret = sess->Input(nread, buf);
        if (nret < 0) {
            if (nret == UV_EOF) {
                LOGV <<  "session pipe error: " << nret << ", close pipe: " << sess->GetKey();
            } else {
                LOGE  << "session pipe error: " << nret << ", close pipe: " << sess->GetKey();
            }
            RemovePipe(sess);
        }
    } else if (nread < 0) {
        OnBtmPipeError(mBtmPipe, nread);
    } else {
        LOGV << "broken data, nread: " << nread;
    }
    return nret;
}

int BridgePipe::RemovePipe(ISessionPipe *pipe) {
    auto it = mTopPipes.find(pipe->GetKey());
    if (it == mTopPipes.end()) {
        LOGE << "pipe " << pipe->GetKey() << " don't belong to bridge pipe.";
    }
    doRemove(it);

    return 0;
}

int BridgePipe::removeAll() {
    for (auto it = mTopPipes.begin(); it != mTopPipes.end(); it++) {
        doRemove(it);
    }
    return 0;
}

int BridgePipe::doRemove(std::map<ISessionPipe::KeyType, ISessionPipe *>::iterator it) {
    if (it != mTopPipes.end()) {
        IPipe *pipe = it->second;
        pipe->SetOutputCb(nullptr);
        pipe->SetOnErrCb(nullptr);

        mErrPipes.insert(*it);
    }
    return 0;
}

ISessionPipe *BridgePipe::FindPipe(const ISessionPipe::KeyType &key) const {
    auto it = mTopPipes.find(key);
    return it != mTopPipes.end() ? it->second : nullptr;
}

void BridgePipe::Start() {
    IPipe::Start();
    mBtmPipe->Start();
}

// if (key) must be true
int BridgePipe::AddPipe(ISessionPipe *pipe) {
    if (pipe && !pipe->GetKey().empty()) {
        const auto &key = pipe->GetKey();

        auto ret = mTopPipes.insert({key, pipe});
        LOGV << "add pipe: " << pipe->GetKey();
        if (!ret.second) {
            LOGE << "insert failed. duplicate pipe";
            return -1;
        }

        pipe->Init();
        pipe->Start();
        auto out = std::bind(&BridgePipe::PSend, this, pipe, std::placeholders::_1, std::placeholders::_2);
        pipe->SetOutputCb(out);

        // no need cast. caputure parameter pipe
        pipe->SetOnErrCb([this](IPipe *p, int err) {
            ISessionPipe *sess = dynamic_cast<ISessionPipe *>(p);
            if (sess) {
                this->OnTopPipeError(sess, err);
            } else {
                LOGE << "dynamic_cast failed for pipe: " << p;
            }
        });
    }
    return 0;
}

void BridgePipe::Flush(uint32_t curr) {
    cleanErrPipes();
    mBtmPipe->Flush(curr);  // normall this will not be necessary if btm pipe input data passively from other soruce

    cleanErrPipes();
    for (auto it = mTopPipes.begin(); it != mTopPipes.end(); it++) {
        it->second->Flush(curr);
    }
//    for (auto &e: mTopPipes) {
//        e.second->Flush(curr);
//    }
    cleanErrPipes();
}

void BridgePipe::cleanErrPipes() {
    for (auto &e: mErrPipes) {
        LOGV << "deleting pipe: " << e.second->GetKey();
        mTopPipes.erase(e.first);
        e.second->Close();
        delete e.second;
    }
    mErrPipes.clear();
}

void BridgePipe::OnTopPipeError(ISessionPipe *pipe, int err) {
    if (err == UV_EOF) {
        LOGV << "pipe " << pipe->GetKey() << " error: " << uv_strerror(err);
    } else {
        LOGE << "pipe " << pipe->GetKey() << " error: " << uv_strerror(err);
    }
    RemovePipe(pipe);
}

void BridgePipe::OnBtmPipeError(IPipe *pipe, int err) {
    assert(pipe == mBtmPipe);   // typically sendto will not fail
    LOGE << "btm pipe error " << err << ": " << uv_strerror(err);
    OnError(this, err);
}

int BridgePipe::PSend(ISessionPipe *pipe, ssize_t nread, const rbuf_t *buf) {
    if (nread > 0) {
        return Send(nread, buf);
    } else if (nread < 0) {    // error occurs. eof or other
        OnTopPipeError(pipe, nread);
        return nread;
    }

    return 0;
}

ISessionPipe *BridgePipe::onCreateNewPipe(const ISessionPipe::KeyType &key, void *addr) {
    if (mCreateNewPipeCb) {
        return mCreateNewPipeCb(key, addr);
    }
    return nullptr;
}

void BridgePipe::SetOnCreateNewPipeCb(const OnCreatePipeCb &cb) {
    mCreateNewPipeCb = cb;
}

