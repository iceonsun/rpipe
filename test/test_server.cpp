//
// Created on 1/7/18.
//

#include "../server/rserver_main.h"

int main(int argc, char **argv) {
    char *fakearg[10] = {nullptr};

    fakearg[0] = argv[0];
    fakearg[1] = "-t47.95.217.247:1984 ";
    fakearg[2] = "-l:20000";
//    fakearg[1] = "-t127.0.0.1:20001";
//    fakearg[2] = "-l:20000";
    return rserver_main(3, fakearg);
}