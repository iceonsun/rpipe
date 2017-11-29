//
// Created on 11/28/17.
//

#include <syslog.h>
#include "UdpBtmPipe.h"
#include "debug.h"

UdpBtmPipe::UdpBtmPipe(int fd, uv_loop_t *loop) : BtmPipe(fd, loop) {

}

int UdpBtmPipe::outputCb(ssize_t nread, const rbuf_t *buf) {
    if (nread > 0 && buf->data) {
        int udp = GetFd();
        auto *addr = static_cast<sockaddr_in *>(buf->data);
        socklen_t sockLen = sizeof(struct sockaddr_in);
        ssize_t nLeft = nread;
        while (nLeft > 0) {
            ssize_t n = sendto(udp, buf->base, nread, 0, reinterpret_cast<const sockaddr *>(addr), sockLen);
            debug(LOG_ERR, "send %d bytes to %s:%d", n, inet_ntoa(addr->sin_addr), ntohs(addr->sin_port));
            nLeft -= n;
        }

        struct sockaddr_in selfAddr;
        socklen_t selfSockLen = sizeof(selfAddr);
        getsockname(udp, reinterpret_cast<sockaddr *>(&selfAddr), &selfSockLen);

        return nread - nLeft;
    }
    return 0;
}

int UdpBtmPipe::onReadable(uv_poll_t *handle) {
    char base[PIPE_BUFSIZ] = {0};
    struct sockaddr_in addr;
    rbuf_t rbuf;
    rbuf.base = base;
    rbuf.data = &addr;
    int nread = 0;

    socklen_t len = sizeof(addr);
//    while ((nread = recvfrom(fd, base, PIPE_BUFSIZ, 0, reinterpret_cast<sockaddr *>(&addr), &len)) > 0) {
    int fd = GetFd();
    while ((nread = recvfrom(fd, base, PIPE_BUFSIZ, 0, reinterpret_cast<sockaddr *>(&addr), &len)) > 0) {
        debug(LOG_ERR, "read %d bytes from %s:%d", nread, inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
        Input(nread, &rbuf);
    }
    return 0;
}
