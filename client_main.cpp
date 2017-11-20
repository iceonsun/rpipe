//
// Created on 11/19/17.
//

#include "RClient.h"

int main(int argc, char **argv) {
    RApp *app = new RClient();
    return app->Main(argc, argv);
}
