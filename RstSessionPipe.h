//
// Created on 11/29/17.
//

#ifndef RPIPE_RSTSESSIONPIPE_H
#define RPIPE_RSTSESSIONPIPE_H

#include "ISessionPipe.h"

class RstSessionPipe : public ISessionPipe {
public:
    RstSessionPipe(IPipe *pipe, const KeyType &key, const sockaddr_in *target);

    int Input(ssize_t nread, const rbuf_t *buf) override;

    int Send(ssize_t nread, const rbuf_t *buf) override;
};


#endif //RPIPE_RSTSESSIONPIPE_H
