//
// Created on 12/4/17.
//

#ifndef RPIPE_STREAMRDWRITER_H
#define RPIPE_STREAMRDWRITER_H


#include <uv.h>
#include "IRdWriter.h"

class TcpRdWriter : public IRdWriter {
public:
    explicit TcpRdWriter(uv_stream_t *stream);
    explicit TcpRdWriter(int fd, uv_loop_t *loop);

    virtual ~TcpRdWriter();

    int Read(char *buf, int len, int *err) override;

    int Write(const char *data, int len, int *err) override;

    int Close() override;

protected:
    virtual int openStream(uv_stream_t *stream, int fd);

    static void write_cb(uv_write_t *write, int status);

private:
    void do_init(uv_stream_t *stream, int fd);

private:
    int mFd = -1;
    uv_stream_t *mStream = nullptr;
};


#endif //RPIPE_STREAMRDWRITER_H
