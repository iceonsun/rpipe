//
// Created on 1/7/18.
//

#include "RServer.h"
#include "rserver_main.h"

int rserver_main(int argc, char **argv) {
    RApp *app = new RServer();
    int nret = app->Parse(argc, argv);
    if (!nret) {
        if (!(nret = app->Init())) {
            nret = app->Start();
        }
    }
    app->Close();
    delete app;
    return nret;
}