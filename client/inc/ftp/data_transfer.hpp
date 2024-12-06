#pragma once
#include <string>

namespace FTP_Client {

class DataTransfer {
public:
    DataTransfer();
    ~DataTransfer();

    bool establish(const std::string& ip, int port);
    bool receive_data(std::string& data);
    bool send_data(const std::string& data);

private:
    int socket_fd;
};

}
