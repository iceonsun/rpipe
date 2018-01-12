//
// Created on 11/15/17.
//

#ifndef RPIPE_RAPP_H
#define RPIPE_RAPP_H

#include "uv.h"
#include "Config.h"
#include "BridgePipe.h"
#include "nmq/INMQPipe.h"

namespace plog {
    class IAppender;
}

class RApp {
public:
    virtual ~RApp() = default;

    virtual int Parse(int argc, char **argv);

    virtual int Init();

    int Start();

    virtual bool isServer() = 0;

    virtual void Close();

    virtual void Flush();

    virtual BridgePipe *CreateBridgePipe(const Config &conf, IPipe *btmPipe, uv_loop_t *loop) = 0;

    virtual IPipe *CreateBtmPipe(const Config &conf, uv_loop_t *loop) = 0;

    const Config &GetConfig();

    uv_loop_t *GetLoop();

    BridgePipe *GetBridgePipe();

    const sockaddr_in *GetTarget();

    static INMQPipe * NewNMQPipeFromConf(IUINT32 conv, const Config &conf, IPipe *top);

private:
    static void flush_cb(uv_timer_t *handle);

private:
    int initLog(const Config &conf);

    int doInit();

    int makeDaemon();

private:
    BridgePipe *mBridge = nullptr;
    uv_timer_t *mFlushTimer = nullptr;
    plog::IAppender *mFileAppender = nullptr;
    plog::IAppender *mConsoleAppender = nullptr;
    Config mConf;
    uv_loop_t *mLoop = nullptr;
    struct sockaddr_in mTargetAddr = {0};
};


#endif //RPIPE_RAPP_H
