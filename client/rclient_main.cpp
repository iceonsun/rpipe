//
// Created on 1/7/18.
//

#include "../RApp.h"
#include "RClient.h"

int rclient_main(int argc, char **argv) {
    RApp *app = new RClient();
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