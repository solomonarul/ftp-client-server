#include "../inc/ftp/session.hpp"
#include <iostream>

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cerr << "Usage: .build/ftp-client <IP> <Port>\n";
        return EXIT_FAILURE;
    }
    const std::string server_ip = argv[1];
    const int server_port = std::stoi(argv[2]);

    FTP_Client::Session session(server_ip, server_port);
    if (!session.connect()) {
        std::cerr << "Failed to connect to the server.\n";
        return EXIT_FAILURE;
    }

    session.start();
    return EXIT_SUCCESS;
}
