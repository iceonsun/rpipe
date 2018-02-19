//
// Created on 1/7/18.
//

#include "../server/rserver_main.h"

int main(int argc, char **argv) {
    char *fakearg[10] = {nullptr};

    fakearg[0] = argv[0];
//    fakearg[1] = "-t127.0.0.1:10010";
        fakearg[1] = "-t47.95.217.247:1984";
    fakearg[2] = "-l:30000";
//    fakearg[1] = "-t127.0.0.1:20001";
//    fakearg[2] = "-l:20000";
    fakearg[3] = "--log=./test_server.log";
    fakearg[4] = "-v";
    fakearg[5] = "--sndwnd=1000";
    fakearg[6] = "--daemon=0";
    return rserver_main(7, fakearg);
}