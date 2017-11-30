//
// Created on 11/15/17.
//

#ifndef RPIPE_RAPP_H
#define RPIPE_RAPP_H


#include "Config.h"
#include "BridgePipe.h"

class RApp {
public:
    int Main(int argc, char **argv);

    virtual int Loop(Config &conf) = 0;

    virtual bool isServer() = 0;

    virtual void Close() = 0;

    virtual void Flush() = 0;

//    virtual uv_handle_t * CreateListenHandle(const Config &conf) = 0;

    virtual BridgePipe * CreateBridgePipe(const Config &conf) = 0;

    virtual IPipe *CreateBtmPipe(const Config &conf) = 0;

    const static int RBUF_SIZE = 1450;

protected:
    static void flush_cb(uv_timer_t *handle);

};


#endif //RPIPE_RAPP_H
