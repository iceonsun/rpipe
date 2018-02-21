//
// Created on 11/15/17.
//

#ifndef RPIPE_RAPP_H
#define RPIPE_RAPP_H

#include "uv.h"
#include "Config.h"
#include "BridgePipe.h"

class INMQPipe;

namespace plog {
    class IAppender;
}

class RApp {
public:
    virtual ~RApp();

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

    static INMQPipe *NewNMQPipeFromConf(uint32_t conv, const Config &conf, IPipe *top);

    static const int SIG_EXIT = SIGUSR1;

protected:
    virtual void onExitSignal();

    virtual void watchExitSignal();

    static void close_signal_handler(uv_signal_t *handle, int signum);

private:
    static void flush_cb(uv_timer_t *handle);

private:
    int initLog(const Config &conf);

    int doInit();

    int makeDaemon();

protected:
    uv_loop_t *mLoop = nullptr;

private:
    BridgePipe *mBridge = nullptr;
    uv_timer_t *mFlushTimer = nullptr;
    plog::IAppender *mFileAppender = nullptr;
    plog::IAppender *mConsoleAppender = nullptr;
    Config mConf;
    struct sockaddr_in mTargetAddr = {0};
    uv_signal_t *mExitSig = nullptr;
};


#endif //RPIPE_RAPP_H
