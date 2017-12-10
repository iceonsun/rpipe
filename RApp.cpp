//
// Created on 11/15/17.
//

#include <syslog.h>
#include "RApp.h"
#include "debug.h"

int RApp::Main(int argc, char **argv) {
    Config conf;
    int ret = conf.parse(isServer(), argc, argv);

    if (ret) {
        debug(LOG_ERR, "failed to parse command line.");
        return ret;
    }

    return Loop(conf);
}

void RApp::flush_cb(uv_timer_t *handle) {
    RApp *app = static_cast<RApp *>(handle->data);
    app->Flush();
}

