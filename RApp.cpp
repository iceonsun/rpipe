//
// Created on 11/15/17.
//

#include "RApp.h"

int RApp::Main(int argc, char **argv) {
    Config conf;
    int ret = conf.parse(isServer(), argc, argv);

//todo: remove this when release
    if (isServer())
        if (ret) {
            return ret;
        }

    return Loop(conf);
}

void RApp::flush_cb(uv_timer_t *handle) {
    RApp *app = static_cast<RApp *>(handle->data);
    app->Flush();
}

