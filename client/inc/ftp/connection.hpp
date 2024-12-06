#pragma once
#include <string>

namespace FTP_Client {

class Connection {
public:
    Connection(const std::string& ip, int port);
    bool connect();
    bool send(const std::string& command);
    bool receive(std::string& response);

private:
    std::string server_ip;
    int server_port;
    int socket_fd;
};

}
