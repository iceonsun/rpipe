//
// Created on 11/20/17.
//

#include <syslog.h>
#include "../../BridgePipe.h"
#include "EchoBtmPipe.h"
#include "../../debug.h"
#include "../../NMQPipe.h"
#include "RawPipe.h"
#include "../../SessionPipe.h"

#define IFACE "127.0.0.1"
#define LOCAL_TCP_PORT 10008


uv_tcp_t tcp;
uv_loop_t *LOOP;
uv_timer_t writeTimer;
uv_timer_t flushTimer;
//uv_tcp_t writer;
int cnt = 0;

RawPipe *raw;
BridgePipe *bridge;

//void close_cb(uv_handle_t *handle) {
//    free(handle);
//}

//void rwrite_cb(uv_write_t *req, int status) {
//    free_rwrite_req(reinterpret_cast<rwrite_req_t *>(req));
//}


//void svr_read_cb(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf) {
//    if (nread < 0) {
//        debug(LOG_ERR, "failed to read: %s\n", uv_strerror(nread));
//        uv_close(reinterpret_cast<uv_handle_t *>(stream), close_cb);
//        free(buf->base);
//    } else if (nread == 0) {
//        free(buf->base);
//    } else {
//        printf("receive %d bytes from server: %.*s\n", nread, nread, buf->base);
//        rwrite_req_t *req = static_cast<rwrite_req_t *>(malloc(sizeof(rwrite_req_t)));
//        req->buf = uv_buf_init(buf->base, nread);
//        uv_write(reinterpret_cast<uv_write_t *>(req), stream, &req->buf, 1, rwrite_cb);
//    }
//}

//void svr_conn_cb(uv_stream_t *server, int status) {
//    if (status) {
//        debug(LOG_ERR, "error: %s\n", uv_strerror(status));
//        uv_close(reinterpret_cast<uv_handle_t *>(server), nullptr);
//        return;
//    }
//
//    uv_tcp_t *client = static_cast<uv_tcp_t *>(malloc(sizeof(uv_tcp_t)));
//    uv_tcp_init(LOOP, client);
//    int nret = uv_accept(server, reinterpret_cast<uv_stream_t *>(client));
//    if (nret) {
//        debug(LOG_ERR, "failed to accept: %s\n", uv_strerror(nret));
//        free(client);
//        return;
//    }
//    uv_read_start(reinterpret_cast<uv_stream_t *>(client), alloc_buf, svr_read_cb);
//}

void timer_cb(uv_timer_t *handle) {
    if (cnt < 10) {
        cnt++;
//        rwrite_req_t *req = static_cast<rwrite_req_t *>(malloc(sizeof(rwrite_req_t)));
//        req->buf.base = static_cast<char *>(malloc(20));
//        int n = sprintf(req->buf.base, "%s:%d", "hello world", cnt);
//        uv_write(reinterpret_cast<uv_write_t *>(req), reinterpret_cast<uv_stream_t *>(&writer), &req->buf, 1,
//                 rwrite_cb);
        rbuf_t out;
        out.base = static_cast<char *>(malloc(sizeof(20)));
        int n = sprintf(out.base, "hello %d", cnt);
        fprintf(stderr, "send %d bytes: %.*s\n", n, n, out.base);
        raw->Send(n, &out);
        free(out.base);
    } else {
        rbuf_t out;
        raw->Send(UV_EOF, &out);    // eof
//        raw->Close();
        uv_timer_stop(&writeTimer);
        uv_timer_stop(&flushTimer);
        bridge->Flush(iclock());
        bridge->Close();
//        uv_timer_stop(&flushTimer);
//        uv_close(reinterpret_cast<uv_handle_t *>(&writer), NULL);
    }
}

void flush_timer_cb(uv_timer_t *handle) {
    bridge->Flush(iclock());
}

int main() {
    IPipe *btm = new EchoBtmPipe();
    bridge = new BridgePipe(btm);
    bridge->Init();

    raw = new RawPipe();
//    raw->SetOnErrCb([](IPipe *pipe, int err) {
//        debug(LOG_ERR, "pipe %p error: %d\n", pipe, err);
//    });
    raw->SetOnRecvCb([](ssize_t size, const rbuf_t *buf) -> int {
        if (size > 0) {
            printf("receive %d bytes: %.*s\n", size, size, buf->base);
        }
        return size;
    });

    NMQPipe *nmqPipe = new NMQPipe(1, nullptr);
    ISessionPipe *sess = new SessionPipe(nmqPipe, uv_default_loop(), 1, nullptr);

//    nmqPipe->SetTargetAddr()
    bridge->AddPipe(sess);

     bridge->Start();

    LOOP = uv_default_loop();
//    uv_tcp_init(LOOP, &tcp);
//    struct sockaddr_in addr = {0};
//    uv_ip4_addr(IFACE, LOCAL_TCP_PORT, &addr);
//    uv_tcp_bind(&tcp, reinterpret_cast<const sockaddr *>(&addr), 0);
//    uv_listen(reinterpret_cast<uv_stream_t *>(&tcp), 5, svr_conn_cb);

    uv_timer_init(LOOP, &writeTimer);
    uv_timer_start(&writeTimer, timer_cb, 500, 1000);

    uv_timer_init(LOOP, &flushTimer);
    uv_timer_start(&flushTimer, flush_timer_cb, 20, 20);

    uv_run(LOOP, UV_RUN_DEFAULT);

//    bridge->Close();
    delete bridge;

    return 0;
}