//
// Created on 1/7/18.
//

#include "../server/rserver_main.h"

#define TADDR "127.0.0.1:20001"
#define LADDR ":20000"

int main(int argc, char **argv) {
    if (argc > 1) {
        return rserver_main(argc, argv);
    }

    char *fakearg[] = {
            argv[0],
            "-t" TADDR,
            "-l" LADDR,
            "--daemon=0",
            "-v",
    };
    return rserver_main(sizeof(fakearg) / sizeof(fakearg[0]), fakearg);
}