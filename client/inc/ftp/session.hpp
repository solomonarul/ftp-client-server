#pragma once
#include "connection.hpp"
#include "command_handler.hpp"

namespace FTP_Client {

class Session {
public:
    Session(const std::string& ip, int port);
    bool connect();
    void start();

private:
    std::string server_ip;
    int server_port;
    Connection connection;
    CommandHandler command_handler;
};

}
