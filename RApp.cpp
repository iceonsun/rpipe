//
// Created on 11/15/17.
//

#include "uv.h"
#include "RApp.h"
#include "util/FdUtil.h"
#include "plog/Log.h"
#include "plog/Appenders/ConsoleAppender.h"
#include "util/ProcUtil.h"

void RApp::flush_cb(uv_timer_t *handle) {
    RApp *app = static_cast<RApp *>(handle->data);
    app->Flush();
}

int RApp::initLog(const Config &conf) {
    if (!conf.log_path.empty()) {
        if (!FdUtil::FileExists(conf.log_path.c_str())) {
            int nret = FdUtil::CreateFile(conf.log_path);
            if (nret < 0) {
                fprintf(stderr, "failed to create log file: %s", strerror(errno));
                return nret;
            }
        }
        mFileAppender = new plog::RollingFileAppender<plog::TxtFormatter>(conf.log_path.c_str(), 100000, 5);
    } else {
        fprintf(stderr, "warning: log path empty\n");
    }

    mConsoleAppender = new plog::ConsoleAppender<plog::TxtFormatter>();
    plog::init(conf.log_level, mConsoleAppender);
    if (mFileAppender) {
        plog::get()->addAppender(mFileAppender);
    }

    return 0;
}

int RApp::Init() {
    if (!mConf.Inited()) {
        fprintf(stderr, "configuration not inited.\n");
        return -1;
    }
    return doInit();
}

int RApp::Parse(int argc, char **argv) {
    return mConf.Parse(isServer(), argc, argv);
}

int RApp::doInit() {
    assert(mConf.Inited());
    mLoop = uv_default_loop();
    int nret = initLog(mConf);
    if (nret) {
        return nret;
    }
//    if (makeDaemon()) {
//        return -1;
//    }

    return 0;
}

int RApp::makeDaemon() {
    ProcUtil::MakeDaemon(mConf.isDaemon); // todo: add daemon test later
    if (mConf.isDaemon) {   // todo: if run in daemon, poll will fail if use default loop (on mac, it's uv__io_check_fd fails). why?
        mLoop = static_cast<uv_loop_t *>(malloc(sizeof(uv_loop_t)));
        memset(mLoop, 0, sizeof(uv_loop_t));
        uv_loop_init(mLoop);
    }
    return 0;
}

int RApp::Start() {
    return Loop(mLoop, mConf);
}