//
// Created on 11/15/17.
//

#ifndef RPIPE_RAPP_H
#define RPIPE_RAPP_H

#include "uv.h"
#include "Config.h"
#include "BridgePipe.h"

namespace plog {
    class IAppender;
}

class RApp {
public:
    virtual ~RApp() = default;

    virtual int Parse(int argc, char **argv);

    virtual int Init();

    virtual int Start();

    virtual int Loop(uv_loop_t *loop, Config &conf) = 0;

    virtual bool isServer() = 0;

    virtual void Close() = 0;

    virtual void Flush() = 0;

//    virtual uv_handle_t * CreateListenHandle(const Config &conf) = 0;

    virtual BridgePipe * CreateBridgePipe(const Config &conf) = 0;

    virtual IPipe *CreateBtmPipe(const Config &conf) = 0;

    const static int RBUF_SIZE = 1450;

protected:
    static void flush_cb(uv_timer_t *handle);

private:
    int initLog(const Config &conf);
    int doInit();
    int makeDaemon();
private:
    plog::IAppender *mFileAppender = nullptr;
    plog::IAppender *mConsoleAppender = nullptr;
    Config mConf;
    uv_loop_t *mLoop = nullptr;
};


#endif //RPIPE_RAPP_H
