//
// Created on 11/15/17.
//

#ifndef RPIPE_CONFIG_H
#define RPIPE_CONFIG_H

#include <string>
#include "json/json11.hpp"

struct Config {
    struct Param {
        Param() = default;
        Param(const Param &param) = default;
        Param &operator=(const Param &param) = default;

        int localListenPort = 10010;
        std::string localListenIface = "127.0.0.1";
        int targetPort = 10011;
        std::string targetIp = "127.0.0.1";
        bool fc = false;
        std::string crypt = "none";    // not used right now
        std::string key = "rpipe123";   // not used right now
//        PIPE_TYPE type;
        int mtu = 1400;             // mtu for nmq
        int sndwnd = 150;         // 1400B * 150
        int rcvwnd = 500;         // 1400B * 2000
        int interval = 20;  //ms
        int tolerance = 2;  // number of segments to cause retransmit. default is 2
        int dupAckLim = 3;  // number of dup acks to think a segment lost
        int backlog = 5;
    };

    enum PIPE_TYPE {
        UDP,
        RAW_TCP
    };

    Config() = default;
    Config(const Config &conf) = default;
    Config &operator=(const Config &conf) = default;

    int parse(bool is_server, int argc, char **argv);

    bool isServer = false;
    bool isDaemon = true;
    Param param;

    static const std::string AES;
    static const int MAX_DUP_ACK_LIMIT = 10;

    json11::Json to_json() const;

public:
    static void ParseJsonFile(Config &config, const std::string &fName, std::string &err);

    static void ParseJsonString(Config &config, const std::string &content, std::string &err);

private:
    static void checkAddr(Param &param, bool is_server);
};

#endif //RPIPE_CONFIG_H
