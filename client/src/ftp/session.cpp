#include "../../inc/ftp/session.hpp"
#include <iostream>

namespace FTP_Client {

Session::Session(const std::string& ip, int port)
    : server_ip(ip), server_port(port), connection(ip, port), command_handler(&connection) {}

bool Session::connect() {
    if (!connection.connect()) return false;
    return command_handler.login();
}

void Session::start() {
    std::string command_line;
    while (true) {
        std::cout << "ftp> ";
        if (!std::getline(std::cin, command_line)) break;
        if (!command_handler.process_command(command_line)) break;
    }
}

}
