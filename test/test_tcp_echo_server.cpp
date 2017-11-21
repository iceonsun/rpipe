//
// Created on 11/19/17.
//

#include <uv.h>
#include <cstdlib>
#include "../rcommon.h"

const static int port = 10022;
const static char *iface = "127.0.0.1";

uv_tcp_t tcpSvr;

void close_cb(uv_handle_t *handle) {
    free(handle);
}

void write_cb(uv_write_t *req, int status) {
    if (status) {
        fprintf(stderr, "failed to write: %s\n", uv_strerror(status));
    }
    free_rwrite_req(reinterpret_cast<rwrite_req_t *>(req));
}

void echo_read(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf) {
    if (nread < 0) {
        free(buf->base);
        fprintf(stderr, "read error: %s\n", uv_strerror(nread));
        uv_close(reinterpret_cast<uv_handle_t *>(stream), close_cb);
    } else if (nread == 0) {
        free(buf->base);
    } else {
        rwrite_req_t *req = static_cast<rwrite_req_t *>(malloc(sizeof(rwrite_req_t)));
        req->buf = uv_buf_init(buf->base, nread);
        uv_write(reinterpret_cast<uv_write_t *>(req), stream, &req->buf, 1, write_cb);
        struct sockaddr_in peer = {0};
        int len = sizeof(peer);
        uv_tcp_getpeername(reinterpret_cast<const uv_tcp_t *>(stream), reinterpret_cast<sockaddr *>(&peer), &len);
        printf("%s:%d: %d bytes: %.*s\n", inet_ntoa(peer.sin_addr), ntohs(peer.sin_port), nread, nread, buf->base);
    }
}

void svr_conn_cb(uv_stream_t *server, int status) {
    if (status) {
        uv_close(reinterpret_cast<uv_handle_t *>(&tcpSvr), nullptr);
    }

    uv_tcp_t *client = static_cast<uv_tcp_t *>(malloc(sizeof(uv_tcp_t)));
    uv_tcp_init(uv_default_loop(), client);
    int nret = uv_accept(server, reinterpret_cast<uv_stream_t *>(client));
    if (nret) {
        fprintf(stderr, "accept error: %s\n", uv_strerror(nret));
        free(client);
    }
    uv_read_start(reinterpret_cast<uv_stream_t *>(client), alloc_buf, echo_read);
}

int main() {
    uv_loop_t *LOOP = uv_default_loop();
    uv_tcp_init(LOOP, &tcpSvr);

    struct sockaddr_in addr = {0};
    uv_ip4_addr(iface, port, &addr);

    int nret = uv_tcp_bind(&tcpSvr, reinterpret_cast<const sockaddr *>(&addr), 0);
    if (nret) {
        fprintf(stderr, "uv_tcp_bind error: %s\n", uv_strerror(nret));
        return 1;
    }

    nret = uv_listen(reinterpret_cast<uv_stream_t *>(&tcpSvr), 5, svr_conn_cb);
    if (nret) {
        fprintf(stderr, "uv_listen error: %s\n", uv_strerror(nret));
        return 2;
    }

    fprintf(stderr, "listening on %s:%d\n", iface, port);

    uv_run(LOOP, UV_RUN_DEFAULT);
}
