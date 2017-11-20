#include <iostream>
#include <util.h>
#include <nmq.h>

IINT32 nmq_output(const char *data, const int len, struct nmq_s *nmq, void *arg) {
    fprintf(stderr, "nmq output: %.*s\n", len, data);
    return len;
}

int main() {
    std::cout << "max(1, 2): = " << MAX(1, 2) << std::endl;
    NMQ *nmq = nmq_new(0, 0);
    nmq_set_output_cb(nmq, nmq_output);
    nmq_send(nmq, "hello", 5);
    nmq_update(nmq, 1);
    nmq_destroy(nmq);
    std::cout << "Hello, World!" << std::endl;
    std::cout << "hello world" + std::to_string(5) << std::endl;
    return 0;
}