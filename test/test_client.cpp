//
// Created on 1/7/18.
//

#include "../client/rclient_main.h"

int main(int argc, char **argv) {
    char *fakearg[10] = {nullptr};
    fakearg[0] = argv[0];
    fakearg[1] = "-l:10010";
//    fakearg[1] = "-l:10010";
//    fakearg[2] = "-t104.131.10.169:20001";
    fakearg[2] = "-t127.0.0.1:30000";
//    fakearg[2] = "-t47.95.217.247:10010";
    fakearg[3] = "--log=./test_client.log";
    fakearg[4] = "--rcvwnd=2000";
    fakearg[5] = "--sndwnd=1000";
    fakearg[6] = "--daemon=0";
    fakearg[7] = "--mtu=1300";
    fakearg[8] = "-v";
    return rclient_main(9, fakearg);
}