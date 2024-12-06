#include "../../inc/ftp/data_transfer.hpp"
#include <arpa/inet.h>
#include <cstring>
#include <unistd.h>
#include <iostream>

namespace FTP_Client {

DataTransfer::DataTransfer() : socket_fd(-1) {}

bool DataTransfer::establish(const std::string& ip, int port) {
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        std::cerr << "Failed to create data transfer socket.\n";
        return false;
    }

    sockaddr_in data_addr{};
    data_addr.sin_family = AF_INET;
    data_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip.c_str(), &data_addr.sin_addr) <= 0) {
        std::cerr << "Invalid data transfer IP address.\n";
        return false;
    }

    if (connect(socket_fd, reinterpret_cast<sockaddr*>(&data_addr), sizeof(data_addr)) < 0) {
        std::cerr << "Failed to connect for data transfer.\n";
        return false;
    }

    return true;
}

bool DataTransfer::receive_data(std::string& data) {
    char buffer[1024];
    data.clear();

    while (true) {
        int bytes_received = recv(socket_fd, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received < 0) {
            std::cerr << "Data transfer receive error.\n";
            return false;
        }
        if (bytes_received == 0) break;

        buffer[bytes_received] = '\0';
        data += buffer;
    }
    return true;
}

bool DataTransfer::send_data(const std::string& data) {
    if (send(socket_fd, data.c_str(), data.length(), 0) < 0) {
        std::cerr << "Data transfer send error.\n";
        return false;
    }
    return true;
}

DataTransfer::~DataTransfer() {
    if (socket_fd >= 0) {
        close(socket_fd);
    }
}

}
