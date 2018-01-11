//
// Created on 1/9/18.
//

#include <unistd.h>
#include <cstdlib>
#include <cstring>
#include "uv.h"
#include "../rcommon.h"

const char *ip = "127.0.0.1";
const int port = 10010;

uv_tcp_t *tcp = nullptr;
//int sock = 0;
int cnt = 0;
void write_cb(uv_write_t* req, int status) {
    free_rwrite_req(reinterpret_cast<rwrite_req_t *>(req));
}

void timer_cb(uv_timer_t *timer) {
    cnt++;

    rwrite_req_t *req = static_cast<rwrite_req_t *>(malloc(sizeof(rwrite_req_t)));
    req->buf.base = static_cast<char *>(malloc(BUFSIZ));
    memset(req->buf.base, 0, BUFSIZ);
    snprintf(req->buf.base, BUFSIZ, "hello world %d\n", cnt);
    req->buf.len = strlen(req->buf.base);

    uv_write(reinterpret_cast<uv_write_t *>(req), reinterpret_cast<uv_stream_t *>(tcp), &req->buf, 1, write_cb);
}

void read_cb(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf) {
    if (nread > 0) {
        fprintf(stderr, "read from server: %.*s", nread, buf->base);
    }
    free(buf->base);
}
int main() {
    uv_loop_t *LOOP = uv_default_loop();
    uv_timer_t timer;
    uv_timer_init(LOOP, &timer);
    uv_timer_start(&timer, timer_cb, 2000, 2000);

    struct sockaddr_in addr = {0};
    uv_ip4_addr(ip, port, &addr);
    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    socklen_t len = sizeof(addr);
    int n = connect(sock, reinterpret_cast<const sockaddr *>(&addr), len);
    if (-1 == n) {
        fprintf(stderr, "connect failed: %s", strerror(errno));
        exit(1);
    }

    tcp = static_cast<uv_tcp_t *>(malloc(sizeof(uv_tcp_t)));
    uv_tcp_init(LOOP, tcp);
    if (0 != (n = uv_tcp_open(tcp, sock))) {
        fprintf(stderr, "uv_tcp_open failed: %s", uv_strerror(n));
        exit(n);
    }
    uv_read_start(reinterpret_cast<uv_stream_t *>(tcp), alloc_buf, read_cb);

    uv_run(LOOP, UV_RUN_DEFAULT);
    return 0;
}