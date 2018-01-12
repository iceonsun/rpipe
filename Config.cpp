//
// Created on 11/15/17.
//

#include <iostream>
#include <fstream>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <plog/Log.h>
#include "Config.h"
#include "thirdparty/args.hxx"
#include "util/FdUtil.h"

using namespace json11;

const std::string Config::AES = "aes";

int Config::Parse(bool is_server, int argc, char **argv) {
    this->isServer = is_server;

    std::string exampleStr = "Example:\n./rclient";
    exampleStr += " -l 127.0.0.1:10010 -t 8.8.8.8:10011\n";
    exampleStr += "./rserver";
    exampleStr += " -l :10011 -t 127.0.0.0:80\n";
    exampleStr += "You are fully responsible for yourself to "
            "use this software.";

    args::ArgumentParser parser("rpipe is a network accelerator.", exampleStr);

    args::HelpFlag help(parser, "help", "Display this help menu", {'h', "help"});
    args::ValueFlag<std::string> localAddr(parser, "", "The local listened IP and port. (default :10010)",
                                           {'l', "localaddr"});
    args::ValueFlag<std::string> targetAddr(parser, "",
                                            "The target IP and port. （default targetIP:10011)",
                                            {'t', "targetaddr"}, args::Options::Required);
    args::ValueFlag<std::string> crypt(parser, "crypt", "Encryption methods. (default aes. Current no cryption works.)",
                                       {"crypt"});
    args::ValueFlag<std::string> key(parser, "key", "Key to encrpt. (default: rpipe123)", {"key"});
    args::ValueFlag<int> mtu(parser, "mtu", "mtu for nmq. (default 1400)", {"mtu"});
    args::ValueFlag<int> sndwnd(parser, "sndwnd", "snd wnd for nmq. (default 150)", {"sndwnd"});
    args::ValueFlag<int> rcvwnd(parser, "rcvwnd", "rcv wnd for nmq. (default 500)", {"rcvwnd"});
    args::ValueFlag<int> interval(parser, "ms", "Flush interval for nmq. (default 20ms)", {"interval"});
    args::ValueFlag<int> tolerance(parser, "tolerance",
                                   "Number of segments to reduce cwnd when flow control is turned on. (default 2)",
                                   {"tolerance"});
    args::ValueFlag<int> ackLim(parser, "ackLimit", "Number of duplicate acks to cause retransmit.", {"ack"});
    args::ValueFlag<int> backlog(parser, "backlog", "backlog of server listen. (default 5)", {"backlog"});

    args::ValueFlag<int> fcOn(parser, "fc", "flow control. 0 for off. other for on (default off)", {"fc"});
    args::ValueFlag<int> daemon(parser, "daemon", "1 for running as daemon, 0 for not. (default as daemon)",
                                {"daemon"});
    args::ValueFlag<std::string> fjson(parser, "/path/to/json_config", "json config file", {"json"});
    args::ValueFlag<std::string> flog(parser, "/path/to/log_file", "log file", {"log"});
    args::Flag verbose(parser, "verbose", "verbose log mode", {'v'});

    try {
        parser.ParseCLI(argc, reinterpret_cast<const char *const *>(argv));

        do {
            // parse json if passed json file
            if (fjson) {
                LOGD << "json file path: " << fjson.Get();
                std::string err;
                ParseJsonFile(*this, fjson.Get(), err);
                if (!err.empty()) {
                    throw args::Error(err);
                }
                break;
            }

            const auto parseAddr = [](const std::string &addr, std::string &ip, int &port) -> bool {
                auto pos = addr.find(':');
                if (pos != std::string::npos && pos < addr.size() - 1) {
                    ip = addr.substr(0, pos);
                    port = std::stoi(addr.substr(pos + 1));
                    if (port > 0) {
                        return true;
                    }
                }
                return false;
            };

            // local ip and port
            if (localAddr) {
                if (!parseAddr(localAddr.Get(), param.localListenIface, param.localListenPort)) {
                    throw args::Error("failed to parse local address. e.g. :10010");
                }
            }

            // target ip and port
            if (targetAddr) {
                if (!parseAddr(targetAddr.Get(), param.targetIp, param.targetPort)) {
                    throw args::Error("failed to parse target addrres. e.g. :10011");
                }
            }

            if (fcOn) {
                param.fc = (fcOn.Get() != 0);
            }

            if (crypt) {
                if (std::string(AES) != crypt.Get()) {
                    throw args::Error("No encryption works now.");
                }
                param.crypt = crypt.Get();
            }

            if (key) {
                param.key = key.Get();
            }

            const auto parseInt = [](const int parsewnd, int &wnd, const std::string &err) {
                if (parsewnd <= 0) {
                    throw args::Error(err);
                }
                wnd = parsewnd;
            };

            if (mtu) {
                parseInt(mtu.Get(), param.mtu, "mtu must be greater than 0");
            }

            if (sndwnd) {
                parseInt(sndwnd.Get(), param.sndwnd, "wnd must be greater than 0");
            }

            if (rcvwnd) {
                parseInt(rcvwnd.Get(), param.rcvwnd, "wnd must be greater than 0");
            }

            if (tolerance) {
                parseInt(tolerance.Get(), param.tolerance, "tolerance must be greater than 0");
            }

            if (ackLim) {
                parseInt(ackLim.Get(), param.dupAckLim, "duplicate ack limit must be greater than 0");
                if (param.dupAckLim > MAX_DUP_ACK_LIMIT) {
                    throw args::Error("duplicate ack limit must be less than " + std::to_string(MAX_DUP_ACK_LIMIT));
                }
            }

            if (backlog) {
                parseInt(backlog.Get(), param.backlog, "backlog must be greater than 0");
            }

            if (daemon) {
                this->isDaemon = (daemon.Get() != 0);
            }

            if (flog) {
                this->log_path = flog.Get();
            }

            if (verbose) {
                this->log_level = plog::verbose;
            }
        } while (false);
        checkAddr(param, is_server);
        mInit = true;
        return 0;
    } catch (args::Help &e) {
        std::cout << parser;
    } catch (args::Error &e) {
        std::cerr << e.what() << std::endl << parser;
    }
    return 1;
}

void Config::ParseJsonFile(Config &config, const std::string &fName, std::string &err) {
    if (!FdUtil::FileExists(fName.c_str())) {
        err = "json file " + fName + " not exists";
        return;
    }

    std::stringstream in;
    std::ifstream fin(fName);
    in << fin.rdbuf();
    ParseJsonString(config, in.str(), err);
}

Json Config::to_json() const {
    return Json::object {
            {"server", isServer},
            {"daemon", isDaemon},
            {"verbose", log_level == plog::verbose},
            {"log", log_path},
            {"param",  Json::object {
                    {"localListenPort",  param.localListenPort},
                    {"localListenIface", param.localListenIface},
                    {"targetPort",       param.targetPort},
                    {"targetIp",         param.targetIp},
                    {"fc",               param.fc},
                    {"crypt",            param.crypt},
                    {"key",              param.key},
                    {"mtu",              param.mtu},
                    {"sndwnd",           param.sndwnd},
                    {"rcvwnd",           param.rcvwnd},
                    {"interval",         param.interval},
                    {"tolerance",        param.tolerance},
                    {"dupAckLim",        param.dupAckLim},
                    {"backlog",          param.backlog},
            }},

    };

}

void Config::ParseJsonString(Config &c, const std::string &content, std::string &err) {
    Json json = Json::parse(content, err);
    if (!err.empty()) {
        return;
    }

    if (json["daemon"].is_number()) {
        c.isDaemon = (json["daemon"].int_value() != 0);
    }

    if (json["verbose"].is_bool()) {
        bool verbose = json["verbose"].bool_value();
        c.log_level = verbose ? plog::verbose : plog::debug;
    }

    if (json["log"].is_string()) {
        c.log_path = json["log"].string_value();
    }

    if (json["param"].is_object()) {
        Param &p = c.param;
        Json::object o = json["param"].object_items();
        if (o["localListenPort"].is_number()) {
            p.localListenPort = o["localListenPort"].int_value();
        }
        if (o["localListenIface"].is_string()) {
            p.localListenIface = o["localListenIface"].string_value();
        }
        if (o["targetPort"].is_number()) {
            p.targetPort = o["targetPort"].int_value();
        }
        if (o["targetIp"].is_string()) {
            p.targetIp = o["targetIp"].string_value();
        }
        if (o["fc"].is_bool()) {
            p.fc = o["fc"].bool_value();
        }
        if (o["crypt"].is_string()) {
            p.crypt = o["crypt"].string_value();
        }
        if (o["key"].is_string()) {
            p.key = o["key"].string_value();
        }
        if (o["mtu"].is_number()) {
            p.mtu = o["mtu"].int_value();
        }
        if (o["sndwnd"].is_number()) {
            p.sndwnd = o["sndwnd"].int_value();
        }
        if (o["rcvwnd"].is_number()) {
            p.rcvwnd = o["rcvwnd"].int_value();
        }
        if (o["interval"].is_number()) {
            p.interval = o["interval"].int_value();
        }
        if (o["tolerance"].is_number()) {
            p.tolerance = o["tolerance"].int_value();
        }
        if (o["dupAckLim"].is_number()) {
            p.dupAckLim = o["dupAckLim"].int_value();
        }
        if (o["backlog"].is_number()) {
            p.backlog = o["backlog"].int_value();
        }
    }
}

void Config::checkAddr(Config::Param &param, bool is_server) {
    if (param.localListenIface.empty()) {
        if (is_server) {
            param.localListenIface = "0.0.0.0";
        } else {
            param.localListenIface = "127.0.0.1";
        }
    }

    if (param.localListenPort == 0) {
        if (is_server) {
            param.localListenPort = SERVER_DEFAULT_LISTEN_PORT;
        } else {
            param.localListenPort = CLIENT_DEFAULT_LISTEN_PORT;
        }
    }

    if (param.targetIp.empty()) {
        if (is_server) {
            param.targetIp = "127.0.0.1";
        } else {
            throw args::Error("No target ip provided for client!");
        }
    }

    if (param.targetPort == 0) {
        if (is_server == false) {
            param.targetPort = SERVER_DEFAULT_LISTEN_PORT;
        }
    }

    auto fn = [](const std::string &ip, int port) -> bool {
        struct sockaddr_in addr = {0};
        if (inet_pton(AF_INET, ip.c_str(), &addr) == 1) {
            if (port > 0 && port <= 65535) {
                return true;
            }
        }
        return false;
    };

    std::string err;
    if (!fn(param.localListenIface, param.localListenPort)) {
        err = "invalid address format " + param.localListenIface + ":" + std::to_string(param.localListenPort);
    }
    if (!fn(param.targetIp, param.targetPort)) {
        err = "invalid address format " + param.targetIp + ":" + std::to_string(param.targetPort);
    }

    if (!err.empty()) {
        throw args::Error(err);
    }
}

void Config::SetInited(bool init) {
    mInit = init;
}

bool Config::Inited() {
    return mInit;
}
