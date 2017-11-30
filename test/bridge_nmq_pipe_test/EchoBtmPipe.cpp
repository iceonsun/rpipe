//
// Created on 11/20/17.
//

#include <syslog.h>
#include "EchoBtmPipe.h"
#include "../../debug.h"

int EchoBtmPipe::Send(ssize_t nread, const rbuf_t *buf) {
    if (nread > 0) {
        rbuf_t *rbuf = static_cast<rbuf_t *>(malloc(sizeof(rbuf_t)));
        rbuf->base = static_cast<char *>(malloc(nread));
//        debug(LOG_INFO, "buf.base: %p. malloc rbuf.base: %p, pthread_t: %d\n", buf->base, rbuf->base, pthread_self());
        memcpy(rbuf->base, buf->base, nread);
        rbuf->len = nread;
        rbuf->data = buf->data; // it's addr will not be freed
        mPendingData.push_back(rbuf);
        debug(LOG_INFO, "send %d bytes.", nread);
//        debug(LOG_INFO, "mPendingSize: %d\n", mPendingData.size());

        cnt++;
    }
    return nread;
}

int EchoBtmPipe::Close() {
    debug(LOG_INFO, "cnt = %d\n", cnt);
    return 0;
}

void EchoBtmPipe::Flush(IUINT32 curr) {
//    debug(LOG_INFO, "mPendingSize: %d, pthread_t: %d\n", mPendingData.size(), pthread_self());
    // this may cause memory leaks. because during foreach, there will come new data thus change size of vector.  foreach will not visit but clear vector after loop
    //
    for (int i = 0; i < mPendingData.size(); i++) {
        rbuf_t *buf = mPendingData[i];
        OnRecv(buf->len, buf);
//        debug(LOG_INFO, "free buf.base: %p\n", buf->base);
        free_rbuf(buf);
    }
    mPendingData.clear();
//    debug(LOG_INFO, "mPendingSize: %d, pthread_t: %d\n", mPendingData.size(), pthread_self()); // todo: why there is memory leaks
}