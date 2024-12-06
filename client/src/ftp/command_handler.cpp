#include "../../inc/ftp/command_handler.hpp"
#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>

namespace FTP_Client {

CommandHandler::CommandHandler(Connection* conn) : connection(conn) {}

bool CommandHandler::login() {
    std::string response;
    if (!connection->receive(response)) {
        return false;
    }
    std::cout << response;

    std::string username;
    std::cout << "Username: ";
    std::getline(std::cin, username);
    connection->send("USER " + username);

    if (!connection->receive(response)) {
        return false;
    }
    std::cout << response;

    std::string password;
    std::cout << "Password: ";
    std::getline(std::cin, password);
    connection->send("PASS " + password);

    if (!connection->receive(response)) {
        return false;
    }
    std::cout << response;

    return response.substr(0, 3) == "230";
}

bool CommandHandler::process_command(const std::string& command_line) {
    std::istringstream iss(command_line);
    std::string command, args;
    iss >> command;
    std::getline(iss, args);

    if (command == "ls") return handle_ls();
    if (command == "cd") return handle_cd(args);
    if (command == "pwd") return handle_pwd();
    if (command == "get") return handle_get(args);
    if (command == "put") return handle_put(args);
    if (command == "quit") return false;

    std::cout << "Unknown command.\n";
    return true;
}

bool CommandHandler::handle_ls() {
    connection->send("PASV");
    std::string response;
    if (!connection->receive(response)) return false;

    std::string ip;
    int port;
    if (!parse_pasv_response(response, ip, port)) {
        std::cerr << "Failed to parse PASV response.\n";
        return false;
    }

    DataTransfer data_transfer;
    if (!data_transfer.establish(ip, port)) {
        return false;
    }

    connection->send("LIST");
    if (!connection->receive(response)) return false;

    if (response.substr(0, 3) == "150") {
        std::string data;
        if (data_transfer.receive_data(data)) {
            std::cout << data;
        }
    }
    connection->receive(response);
    return true;
}

bool CommandHandler::handle_cd(const std::string& args) {
    connection->send("CWD " + args);
    std::string response;
    return connection->receive(response) && (std::cout << response, true);
}

bool CommandHandler::handle_pwd() {
    connection->send("PWD");
    std::string response;
    return connection->receive(response) && (std::cout << response, true);
}

bool CommandHandler::handle_get(const std::string& filename) {
    connection->send("PASV");
    std::string response;
    if (!connection->receive(response)) return false;

    std::string ip;
    int port;
    if (!parse_pasv_response(response, ip, port)) {
        std::cerr << "Failed to parse PASV response.\n";
        return false;
    }

    DataTransfer data_transfer;
    if (!data_transfer.establish(ip, port)) {
        return false;
    }

    connection->send("RETR " + filename);
    if (!connection->receive(response)) return false;

    if (response.substr(0, 3) == "150") {
        std::string data;
        if (data_transfer.receive_data(data)) {
            std::ofstream file(filename, std::ios::binary);
            file << data;
            file.close();
        }
    }
    connection->receive(response);
    return true;
}

bool CommandHandler::handle_put(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open file.\n";
        return false;
    }

    connection->send("PASV");
    std::string response;
    if (!connection->receive(response)) return false;

    std::string ip;
    int port;
    if (!parse_pasv_response(response, ip, port)) {
        std::cerr << "Failed to parse PASV response.\n";
        return false;
    }

    DataTransfer data_transfer;
    if (!data_transfer.establish(ip, port)) {
        return false;
    }

    connection->send("STOR " + filename);
    if (!connection->receive(response)) return false;

    if (response.substr(0, 3) == "150") {
        std::ostringstream buffer;
        buffer << file.rdbuf();
        std::string data = buffer.str();
        data_transfer.send_data(data);
    }
    connection->receive(response);
    return true;
}

bool CommandHandler::parse_pasv_response(const std::string& response, std::string& ip, int& port) {
    std::regex pasv_regex(R"(\((\d+),(\d+),(\d+),(\d+),(\d+),(\d+)\))");
    std::smatch match;
    if (std::regex_search(response, match, pasv_regex) && match.size() == 7) {
        ip = match[1].str() + "." + match[2].str() + "." + match[3].str() + "." + match[4].str();
        port = std::stoi(match[5].str()) * 256 + std::stoi(match[6].str());
        return true;
    }
    return false;
}

}
