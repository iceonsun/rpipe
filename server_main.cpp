//
// Created on 11/19/17.
//

#include "RServer.h"

int main(int argc, char **argv) {
    RApp *app = new RServer();
    return app->Main(argc, argv);
}
