//
// Created on 11/21/17.
//

#include <cassert>
#include <syslog.h>
#include <sstream>
#include <nmq.h>
#include "SessionPipe.h"
#include "thirdparty/debug.h"

SessionPipe::SessionPipe(IPipe *pipe, uv_loop_t *loop, const KeyType &key, const sockaddr_in *target)
        : ISessionPipe(pipe, key, target) {
    mLoop = loop;
}

SessionPipe::SessionPipe(IPipe *pipe, uv_loop_t *loop, int conv, const sockaddr_in *target)
        : SessionPipe(pipe, loop, BuildKey(conv, target), target) {
}

SessionPipe::~SessionPipe() {
    debug(LOG_ERR, "");
    assert(mRepeatTimer == nullptr);
}

void SessionPipe::SetExpireIfNoOps(IUINT32 sec) {
    assert(sec >= MIN_EXPIRE_SEC && sec < MAX_EXPIRE_SEC);
    if (mRepeatTimer) {
        debug(LOG_ERR, "timer running.");
        return;
    }
    debug(LOG_ERR, "set timeout. sec: %d. current: %d", sec, time(0));

    assert(mLoop != nullptr);
    mRepeatTimer = new RTimer(mLoop);

    if (sec > 0) {
        auto fn = std::bind(&SessionPipe::timer_cb, this, std::placeholders::_1);
        mRepeatTimer->Start(sec * 1000, sec * 1000, fn, nullptr);
    }
}

void SessionPipe::timer_cb(void *arg) {
    debug(LOG_ERR, "timeout. current: %d", time(0));
    if (mCnt == mLastCnt) { // no data flow during last period
        timeoutToClose();
    }
    mLastCnt = mCnt;
}

int SessionPipe::Input(ssize_t nread, const rbuf_t *buf) {
    mCnt++;
    assert(nread >= 0);
    if (nread >= HEAD_LEN) {
        char cmd;
        IUINT32 conv;
        const auto key = BuildKey(nread, buf);
        const char *p = decodeHead(buf->base, nread, &cmd, &conv);
        int headLen = p - buf->base;
        debug(LOG_ERR, "sess: %s cmd: %d", mKey.c_str(), cmd);
        if (key != mKey) {
            debug(LOG_ERR, "conv doesn't match");
            return 0;
        } else if (cmd == FIN) {
            onPeerEof();
            return nread;
        } else if (cmd == RST) {
            onPeerRst();
        } else {
            rbuf_t rbuf = {0};
            rbuf.base = const_cast<char *>(p);
            rbuf.data = nullptr;
            if (nread == headLen) {
                return 0;   // 0 bytes. do nothing.
            }
            return ISessionPipe::Input(nread - headLen, &rbuf);
        }
    } else if (nread > 0) {
        debug(LOG_ERR, "broken msg: key: %s, nread: %d", mKey.c_str(), nread);
        return 0;   // broken msg
    }
    return nread;   // we don't process error here. it's processed in bridgepipe
}

int SessionPipe::Send(ssize_t nread, const rbuf_t *buf) {
    mCnt++;
    if (nread > 0) {
        int nret = doSend(nread, buf);
        if (nret < 0) {
            debug(LOG_ERR, "send error: %d", nret);
            OnError(this, nret);
        }
        return nret;
    } else if (nread < 0) {
        debug(LOG_ERR, "err state: %d", nread);
        if (nread == UV_EOF) {
            onSelfEof();
            return nread;
        } else {
            OnError(this, nread);
            return nread;
        }
    }
    return nread;

}

int SessionPipe::Close() {
    ISessionPipe::Close();

    if (mRepeatTimer) {
        mRepeatTimer->Stop();
        delete mRepeatTimer;
        mRepeatTimer = nullptr;
    }

    return 0;
}

void SessionPipe::timeoutToClose() {
    notifyPeerClose();
    OnError(this, UV_EOF);  // timeout error
}

int SessionPipe::doSend(ssize_t nread, const rbuf_t *buf) {
    assert(nread > 0 && buf->base);

    rbuf_t rbuf = {0};
    char base[nread + HEAD_LEN] = {0};
    ssize_t headLen = insertHead(base, nread + HEAD_LEN, (mCnt == 1) ? SYN : NOP, mConv);
    memcpy(base + headLen, buf->base, nread);
    rbuf.base = base;
    rbuf.data = mAddr;
    return ISessionPipe::Send(nread + headLen, &rbuf);
}

// flush is forbidden here. it may cause recursion and stackoverflow
void SessionPipe::onPeerEof() {
    debug(LOG_ERR, "onPeerEof.");
    OnError(this, UV_EOF);  // report error and close
}

void SessionPipe::onSelfEof() {
    debug(LOG_ERR, "onSelfEof. close self. don't close peer. curr: %d", iclock() % 10000);
//    notifyPeerClose();
    OnError(this, UV_EOF);  // 注释掉以后继续发。观察关掉本段后，对面的情况.
}

void SessionPipe::onPeerRst() {
    debug(LOG_ERR, "peer rst");
    OnError(this, UV_EOF);
}
