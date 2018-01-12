//
// Created on 1/7/18.
//

#include "RServerApp.h"
#include "rserver_main.h"

int rserver_main(int argc, char **argv) {
    RApp *app = new RServerApp();
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