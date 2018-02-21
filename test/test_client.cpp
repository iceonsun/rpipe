//
// Created on 1/7/18.
//

#include "../client/rclient_main.h"

#define TADDR "127.0.0.1:20000"
#define LADDR ":10000"

int main(int argc, char **argv) {
    if (argc > 1) {
        return rclient_main(argc, argv);
    }

    char *fakearg[] = {
            argv[0],
            "-t" TADDR,
            "-l" LADDR,
            "--daemon=0",
            "-v",
            "--log=./test_client.log",
    };
    return rclient_main(sizeof(fakearg) / sizeof(fakearg[0]), fakearg);
}