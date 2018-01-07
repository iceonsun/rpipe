//
// Created on 1/7/18.
//

#include "RServer.h"
#include "rserver_main.h"

int rserver_main(int argc, char **argv) {
    RApp *app = new RServer();
    return app->Main(argc, argv);
}