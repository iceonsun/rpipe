//
// Created on 11/15/17.
//

#ifndef RPIPE_CONFIG_H
#define RPIPE_CONFIG_H

#include <string>
#include "thirdparty/json11.hpp"
#include "rpdefs.h"
#include "plog/Severity.h"

struct Config {
    struct Param {
        Param() = default;

        Param(const Param &param) = default;

        Param &operator=(const Param &param) = default;

        int localListenPort = 0;
        std::string localListenIface;
        int targetPort = 0;
        std::string targetIp;
        bool fc = false;
        std::string crypt = "none";    // not used right now
        std::string key = "rpipe123";   // not used right now
        int mtu = 1400;             // mtu for nmq
#ifdef RPIPE_IS_SERVER
        int sndwnd = 500;         // 1400B * 500
        int rcvwnd = 500;         // 1400B * 2000
#else
        int sndwnd = 150;         // 1400B * 100
        int rcvwnd = 500;         // 1400B * 2000
#endif
        int interval = 20;  //ms
        int tolerance = 2;  // number of segments to cause retransmit. default is 2
        int dupAckLim = 3;  // number of dup acks to think a segment lost
        int backlog = 5;
    };

    Config() = default;

    Config(const Config &conf) = default;

    Config &operator=(const Config &conf) = default;

    std::string log_path = RPIPE_LOG_FILE_PATH;
    plog::Severity log_level = plog::debug;

    bool isServer = false;
    bool isDaemon = true;
    Param param;

    static const int SERVER_DEFAULT_LISTEN_PORT = 443;
    static const int CLIENT_DEFAULT_LISTEN_PORT = 10086;
    static const std::string AES;
    static const int MAX_DUP_ACK_LIMIT = 10;

    int Parse(bool is_server, int argc, char **argv);

    json11::Json to_json() const;

    void SetInited(bool init);

    bool Inited();

private:
    bool mInit = false;

public:
    static void ParseJsonFile(Config &config, const std::string &fName, std::string &err);

    static void ParseJsonString(Config &c, const std::string &content, std::string &err);

private:
    static void checkAddr(Param &param, bool is_server);
};

#endif //RPIPE_CONFIG_H
