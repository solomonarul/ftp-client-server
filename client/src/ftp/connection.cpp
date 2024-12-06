#include "ftp/connection.hpp"
#include <arpa/inet.h>
#include <cstring>
#include <unistd.h>
#include <iostream>

namespace FTP_Client {

Connection::Connection(const std::string& ip, int port) 
    : server_ip(ip), server_port(port), socket_fd(-1) {}

bool Connection::connect() {
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        std::cerr << "Failed to create socket.\n";
        return false;
    }

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    if (inet_pton(AF_INET, server_ip.c_str(), &server_addr.sin_addr) <= 0) {
        std::cerr << "Invalid server IP address.\n";
        return false;
    }

    if (::connect(socket_fd, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr)) < 0) {
        std::cerr << "Failed to connect to server.\n";
        return false;
    }

    std::cout << "Connected to server.\n";
    return true;
}

bool Connection::send(const std::string& command) {
    std::string formatted_command = command + "\r\n";
    if (::send(socket_fd, formatted_command.c_str(), formatted_command.length(), 0) < 0) {
        std::cerr << "Failed to send command.\n";
        return false;
    }
    return true;
}

bool Connection::receive(std::string& response) {
    char buffer[1024];
    int bytes_received = ::recv(socket_fd, buffer, sizeof(buffer) - 1, 0);
    if (bytes_received < 0) {
        std::cerr << "Failed to receive response.\n";
        return false;
    }
    buffer[bytes_received] = '\0';
    response = buffer;
    return true;
}

Connection::~Connection() {
    if (socket_fd >= 0) {
        close(socket_fd);
    }
}

}
