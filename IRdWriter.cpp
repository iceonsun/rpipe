//
// Created on 12/4/17.
//

#include "IRdWriter.h"

void IRdWriter::SetOnErrCb(const IRdWriter::IRdWriterCb &cb) {
    mOnErrCb = cb;
}

void IRdWriter::OnErr(int err) {
    if (mOnErrCb) {
        mOnErrCb(err);
    }
}
