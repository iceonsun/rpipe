//
// Created by System Administrator on 1/11/18.
//

#include <cstdlib>
#include <unistd.h>
#include <plog/Log.h>
#include "ProcUtil.h"

int ProcUtil::MakeDaemon(bool d) {
    if (d) {
//        signal(SIGCHLD,SIG_IGN);
        const int nret = fork();
        if (nret == 0) {
            int n = setsid();
            if (-1 == n) {
                LOGE << "make daemon failed. setsid failed " << strerror(errno);
                exit(1);
            }
            LOGI << "Run in background. pid: " << getpid();
            umask(0);
            if (chdir("/") < 0) {
                LOGE << "chdir failed: " << strerror(errno);
                exit(-1);
            }
            for (auto f: {STDIN_FILENO, STDOUT_FILENO, STDERR_FILENO}) {
                close(f);
            }
            int fd = open("/dev/null", O_RDWR);
            dup2(fd, STDOUT_FILENO);
            dup2(fd, STDERR_FILENO);
        } else if (nret > 0) {
            exit(0);    // parent;
        } else {
            LOGE << "make process daemon failed: " << strerror(errno);
            exit(-1);
        }
    }

    return 0;
}
