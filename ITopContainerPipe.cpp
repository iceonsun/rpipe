//
// Created on 11/22/17.
//

#include <cassert>
#include "ITopContainerPipe.h"

using namespace std::placeholders;

ITopContainerPipe::ITopContainerPipe(IPipe *pipe) {
    mTopPipe = pipe;
}

int ITopContainerPipe::Init() {
    IPipe::Init();
    if (mTopPipe) {
        mTopPipe->Init();
        auto sendfn = std::bind(&ITopContainerPipe::Send, this, _1, _2);
        mTopPipe->SetOutputCb(sendfn);

        auto recvfn = std::bind(&IPipe::Input, mTopPipe, _1, _2);
        SetOnRecvCb(recvfn);

        mTopPipe->SetOnErrCb([this] (IPipe *pipe, int err) {
            this->OnError((IPipe*)this, err);
        });
    }

    return 0;
}

int ITopContainerPipe::Close() {
    IPipe::Close();
    if (mTopPipe) {
        mTopPipe->SetOutputCb(nullptr);
//        mTopPipe->SetOnErrCb(nullptr);  a huge bug!!!
        SetOnRecvCb(nullptr);
        mTopPipe->SetOnErrCb(nullptr);

        mTopPipe->Close();

        delete mTopPipe;
        mTopPipe = nullptr;
    }
}

IPipe* ITopContainerPipe::topPipe() {
    return mTopPipe;
}

void ITopContainerPipe::Flush(uint32_t curr) {
    if (mTopPipe) {
        mTopPipe->Flush(curr);
    }
}

void ITopContainerPipe::BlockTop() {
    if (mTopPipe) {
        mTopPipe->SetOutputCb(nullptr);
        SetOnRecvCb(nullptr);
        mTopPipe->SetOnErrCb(nullptr);
    }
}
