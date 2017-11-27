//
// Created on 11/22/17.
//

#include <cassert>
#include "ITopContainerPipe.h"

using namespace std::placeholders;

ITopContainerPipe::ITopContainerPipe(IPipe *pipe) {
    mTopPipe = pipe;
}

ITopContainerPipe::~ITopContainerPipe() {
    assert(mTopPipe == nullptr);
}

int ITopContainerPipe::Init() {
    mTopPipe->Init();
    auto send = std::bind(&ITopContainerPipe::Send, this, _1, _2);
    mTopPipe->SetOutputCb(send);

    auto recv = std::bind(&IPipe::Input, mTopPipe, _1, _2);
    SetOnRecvCb(recv);

    mTopPipe->SetOnErrCb([this] (IPipe *pipe, int err) {
        this->OnError((IPipe*)this, err);
    });

    return IPipe::Init();
}

int ITopContainerPipe::Close() {
    if (mTopPipe) {
        mTopPipe->SetOnErrCb(nullptr);
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

void ITopContainerPipe::Flush(IUINT32 curr) {
    mTopPipe->Flush(curr);
}
