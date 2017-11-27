#include <iostream>
#include <util.h>
#include <nmq.h>
#include <syslog.h>
#include "RTimer.h"
#include "debug.h"
#include "RApp.h"
#include "RServer.h"
#include "RClient.h"

int g_argc;
char **g_argv;

IINT32 nmq_output(const char *data, const int len, struct nmq_s *nmq, void *arg) {
    fprintf(stderr, "nmq output: %.*s\n", len, data);
    return len;
}

void run_app(void *arg) {
    RApp *app = static_cast<RApp *>(arg);
    app->Main(g_argc, g_argv);
}

int main(int argc, char **argv) {
//    g_argc = argc;
//    g_argv = argv;
//
//    uv_thread_t client;
//    uv_thread_t server;
//    RApp *svr = new RServer();
//    RApp *cli = new RClient();
//    uv_thread_create(&client, run_app, cli);
//    uv_thread_create(&server, run_app, svr);
//
//    uv_run(uv_default_loop(), UV_RUN_DEFAULT);
//
//    uv_thread_join(&client);
//    uv_thread_join(&server);
//    delete svr;
//    delete cli;
    return 0;
}