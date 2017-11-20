//
// Created on 11/15/17.
//

#ifndef RPIPE_CONFIG_H
#define RPIPE_CONFIG_H


struct Config {
    struct Param {
        int localListenPort = 10043;
        char *localListenIface; // default "127.0.0.1"
        int targetPort = 10044;
        char* targetIp;
        char *crypt;    // not used right now
        char *passwd;   // not used right now
//        PIPE_TYPE type;
        int MTU = 1500;
        int sndwnd = 2000;         // 1500B * 2000
        int rcvwnd = 2000;         // 1500B * 2000
        int interval = 20;  //ms
        int tolerance = 2;  // number of segments to cause retransmit. default is 2
        int dupAckLim = 3;  // number of dup acks to think a segment lost
        int BACKLOG = 5;
    };

    bool isServer = false;
    bool isDaemon = true;
    Param param;


    enum PIPE_TYPE {
        UDP,
        RAW_TCP
    };

    int parse(int argc, char **argv);
};

#endif //RPIPE_CONFIG_H
