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
int main(int argc, char **argv) {
    uv_tcp_t tcp;
    uv_tcp_init(uv_default_loop(), &tcp);
    return 0;
}