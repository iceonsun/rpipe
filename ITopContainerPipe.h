//
// Created on 11/22/17.
//

#ifndef RPIPE_ITOPPIPE_H
#define RPIPE_ITOPPIPE_H


#include "IPipe.h"

class ITopContainerPipe : public IPipe {
public:
    explicit ITopContainerPipe(IPipe *topPipe);

    int Init() override;

    int Close() override;

    void Flush(IUINT32 curr) override;

    // new methods
    virtual IPipe* topPipe();

    virtual void BlockTop();


private:
    IPipe *mTopPipe = nullptr;
};


#endif //RPIPE_ITOPPIPE_H
