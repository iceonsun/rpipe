//
// Created on 1/7/18.
//

#include "../client/rclient_main.h"

int main(int argc, char **argv) {
    char *fakearg[10] = {nullptr};
    fakearg[0] = argv[0];
    fakearg[1] = "-l:10010";
    fakearg[2] = "-t127.0.0.1:20000";
    fakearg[3] = "--log=./test_client.log";
    return rclient_main(4, fakearg);
}