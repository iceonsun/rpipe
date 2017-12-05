//
// Created on 12/4/17.
//

#include "BtmPipe.h"
#include <sys/socket.h>
#include <cassert>
#include "FdUtil.h"

void FdUtil::CheckDgramFd(int fd) {
    checkFdType(fd, SOCK_DGRAM);
}

void FdUtil::CheckStreamFd(int fd) {
    checkFdType(fd, SOCK_STREAM);
}

void FdUtil::checkFdType(int fd, int type) {
    assert(fd >= 0);
    int currType;
    socklen_t len = sizeof(socklen_t);
    getsockopt(fd, SOL_SOCKET, SO_TYPE, &currType, &len);
    assert(currType == type);
}

int FdUtil::SetNonblocking(int &fd) {
    int oflag = fcntl(fd, F_GETFL, 0);
    int newFlag = oflag | O_NONBLOCK;
    fcntl(fd, F_SETFL, newFlag);
}

void FdUtil::SetBlocking(int &fd) {
    int oflag = fcntl(fd, F_GETFL, 0);
    int newFlag = oflag & (~O_NONBLOCK);
    fcntl(fd, F_SETFL, newFlag);
}