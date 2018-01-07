//
// Created on 1/7/18.
//

#include "../RApp.h"
#include "RClient.h"

int rclient_main(int argc, char **argv) {
    RApp *app = new RClient();
    return app->Main(argc, argv);
}