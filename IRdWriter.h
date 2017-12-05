//
// Created on 12/4/17.
//

#ifndef RPIPE_IREADER_H
#define RPIPE_IREADER_H


#include <functional>

class IRdWriter {
public:

    typedef std::function<void(int)> IRdWriterCb;

    virtual int Read(char *buf, int len, int *err) = 0;

    virtual int Write(const char *data, int len, int *err) = 0;

    virtual int Close() = 0;

    virtual void SetOnErrCb(const IRdWriterCb &cb);

    virtual void OnErr(int err);

private:
    IRdWriterCb
            mOnErrCb = nullptr;
};


#endif //RPIPE_IREADER_H
