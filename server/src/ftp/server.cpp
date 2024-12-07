#include "ftp/server.hpp"

#include <regex>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <iostream>

FTPServer::FTPServer(FTPServerConfig config)
{
    this->config = config;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0)
    {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    // Set the SO_REUSEADDR socket option
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(config.port);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0)
    {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0)
    {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    data_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (data_socket == 0)
    {
        perror("Data socket failed");
        exit(EXIT_FAILURE);
    }

    std::cout << "[INFO]: Server initialized on port " << config.port << "\n";
}

FTPServer::~FTPServer()
{
    close(server_fd);
    close(data_socket);
    std::cout << "[INFO]: Server on port " << config.port << " shut down.\n";
}

void FTPServer::start()
{
    while (true)
    {
        int addrlen = sizeof(address);
        client_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
        if (client_socket < 0)
        {
            perror("Accept failed");
            exit(EXIT_FAILURE);
        }

        // Initialize session for the client
        sessions[client_socket] = {};

        std::cout << "[INFO]: Client connected from " << inet_ntoa(address.sin_addr) << ":" << ntohs(address.sin_port) << "\n";
        sendResponse("220 Server ready.\r\n", client_socket);
        handleClient(client_socket);
    }
}

void FTPServer::handleClient(int client_socket)
{
    char* buffer = (char*)calloc(config.buffer_size, sizeof(char));
    while (true)
    {
        int valread = read(client_socket, buffer, config.buffer_size);
        if (valread <= 0)
        {
            std::cout << "[INFO]: Client disconnected\n";
            sessions.erase(client_socket);
            break;
        }
        std::string command(buffer, valread);
        std::cout << "[TRACE]: Received command: " << command;
        handleCommand(command, client_socket);
    }
    close(client_socket);
    free(buffer);
}

void FTPServer::handlePort(const std::string& params, int client_socket)
{
    std::cout << "[TRACE]: PORT " << params << "\n";

    unsigned int h1, h2, h3, h4, p1, p2;
    if (sscanf(params.c_str(), "%u,%u,%u,%u,%u,%u", &h1, &h2, &h3, &h4, &p1, &p2) != 6)
    {
        sendResponse("501 Syntax error in parameters or arguments.\r\n", client_socket);
        return;
    }

    sockaddr_in &data_addr = sessions[client_socket].active_data_addr;
    data_addr.sin_family = AF_INET;
    data_addr.sin_addr.s_addr = htonl((h1 << 24) | (h2 << 16) | (h3 << 8) | h4);
    data_addr.sin_port = htons((p1 << 8) | p2);

    // Ensure any existing passive connection is closed
    if (sessions[client_socket].passive_mode)
    {
        close(data_socket);
        data_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (data_socket == 0)
        {
            perror("Data socket failed");
            sendResponse("421 Service not available, closing control connection.\r\n", client_socket);
            return;
        }

        // Set the SO_REUSEADDR socket option
        int opt = 1;
        if (setsockopt(data_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
        {
            perror("setsockopt");
            sendResponse("421 Service not available, closing control connection.\r\n", client_socket);
            return;
        }
    }

    sessions[client_socket].passive_mode = false;
    sessions[client_socket].active_mode = true;

    sendResponse("200 Command okay.\r\n", client_socket);
}

void FTPServer::handleCommand(const std::string& command, int client_socket)
{
    std::istringstream iss(command);
    std::string cmd;
    iss >> cmd;

    if (cmd == "USER")
    {
        std::string user;
        iss >> user;
        handleUser(user, client_socket);
    }
    else if (cmd == "PASS")
    {
        std::string pass;
        iss >> pass;
        handlePassword(pass, client_socket);
    }
    else if (cmd == "LIST")
        handleList(client_socket);
    else if (cmd == "STOR")
    {
        std::string filename;
        iss >> filename;
        handleStore(filename, client_socket);
    }
    else if (cmd == "RETR")
    {
        std::string filename;
        iss >> filename;
        handleRetrieve(filename, client_socket);
    }
    else if (cmd == "PASV")
        handlePassive();
    else if (cmd == "PORT")
    {
        std::string params;
        iss >> params;
        handlePort(params, client_socket);
    }
    else if (cmd == "SYST")
        handleSystem(client_socket);
    else if (cmd == "TYPE")
    {
        std::string type;
        iss >> type;
        handleType(type, client_socket);
    }
    else if (cmd == "QUIT")
    {
        sendResponse("221 Goodbye!\r\n", client_socket);
        close(client_socket);
        sessions.erase(client_socket);
    }
    else
        sendResponse("500 Unknown command.\r\n", client_socket);
}

void FTPServer::handleUser(const std::string& user, int client_socket)
{
    std::cout << "[TRACE]: USER " << user << "\n";
    if (user == config.user)
        sendResponse("331 User name okay, need password.\r\n", client_socket);
    else
        sendResponse("530 Not logged in.\r\n", client_socket);
}

void FTPServer::handlePassword(const std::string& pass, int client_socket)
{
    std::cout << "[TRACE]: PASS " << pass << "\n";
    if (pass == config.pass)
    {
        sessions[client_socket].logged_in = true;
        sendResponse("230 User logged in, proceed.\r\n", client_socket);
    }
    else
        sendResponse("530 Not logged in.\r\n", client_socket);
}

void FTPServer::handlePassive()
{
    std::cout << "[TRACE]: PASV\n";

    // Ensure any existing active connection is closed
    if (sessions[client_socket].active_mode)
    {
        close(data_socket);
    }

    data_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (data_socket == 0)
    {
        perror("Data socket failed");
        sendResponse("421 Service not available, closing control connection.\r\n", client_socket);
        return;
    }

    // Set the SO_REUSEADDR socket option
    int opt = 1;
    if (setsockopt(data_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        perror("setsockopt");
        sendResponse("421 Service not available, closing control connection.\r\n", client_socket);
        return;
    }

    int port = config.data_port; // Use the configured data port
    std::string response = "227 Entering Passive Mode (127,0,0,1," + std::to_string(port / 256) + "," + std::to_string(port % 256) + ")\r\n";
    sendResponse(response, client_socket);

    sockaddr_in data_addr;
    data_addr.sin_family = AF_INET;
    data_addr.sin_addr.s_addr = INADDR_ANY;
    data_addr.sin_port = htons(port);

    if (bind(data_socket, (struct sockaddr*)&data_addr, sizeof(data_addr)) == -1)
    {
        perror("bind");
        return;
    }

    if (listen(data_socket, 1) < 0)
    {
        perror("listen");
        return;
    }

    sessions[client_socket].passive_mode = true;
    sessions[client_socket].active_mode = false;
}

void FTPServer::handleList(int client_socket)
{
    if (!sessions[client_socket].logged_in)
    {
        sendResponse("530 Not logged in.\r\n", client_socket);
        return;
    }

    std::cout << "[TRACE]: LIST\n";
    sendResponse("150 File status okay; about to open data connection.\r\n", client_socket);

    int data_fd;
    if (sessions[client_socket].passive_mode)
    {
        sockaddr_in data_addr;
        socklen_t addrlen = sizeof(data_addr);
        data_fd = accept(data_socket, (struct sockaddr*)&data_addr, &addrlen);
        if (data_fd < 0)
        {
            perror("accept");
            sendResponse("425 Can't open data connection.\r\n", client_socket);
            return;
        }
    }
    else if (sessions[client_socket].active_mode)
    {
        data_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (data_fd == -1)
        {
            perror("socket");
            sendResponse("425 Can't open data connection.\r\n", client_socket);
            return;
        }
        if (connect(data_fd, (struct sockaddr*)&sessions[client_socket].active_data_addr, sizeof(sessions[client_socket].active_data_addr)) == -1)
        {
            perror("connect");
            close(data_fd);
            sendResponse("425 Can't open data connection.\r\n", client_socket);
            return;
        }
    }
    else
    {
        sendResponse("425 Can't open data connection.\r\n", client_socket);
        return;
    }

    std::string response;
    DIR* dir;
    struct dirent* ent;
    if ((dir = opendir(sessions[client_socket].current_directory.c_str())) != NULL)
    {
        while ((ent = readdir(dir)) != NULL)
        {
            response += ent->d_name;
            response += "\r\n";
        }
        closedir(dir);
    }
    else
        response = "550 Requested action not taken. Directory unavailable.\r\n";

    send(data_fd, response.c_str(), response.length(), 0);
    close(data_fd);

    sendResponse("226 Closing data connection. Requested file action successful.\r\n", client_socket);
}

void FTPServer::handleStore(const std::string& filename, int client_socket)
{
    if (!sessions[client_socket].logged_in)
    {
        sendResponse("530 Not logged in.\r\n", client_socket);
        return;
    }

    std::cout << "[TRACE]: STOR " << filename << "\n";
    sendResponse("150 File status okay; about to open data connection.\r\n", client_socket);

    int data_fd;
    if (sessions[client_socket].passive_mode)
    {
        sockaddr_in data_addr;
        socklen_t addrlen = sizeof(data_addr);
        data_fd = accept(data_socket, (struct sockaddr*)&data_addr, &addrlen);
        if (data_fd < 0)
        {
            perror("accept");
            sendResponse("425 Can't open data connection.\r\n", client_socket);
            return;
        }
    }
    else if (sessions[client_socket].active_mode)
    {
        data_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (data_fd == -1)
        {
            perror("socket");
            sendResponse("425 Can't open data connection.\r\n", client_socket);
            return;
        }
        if (connect(data_fd, (struct sockaddr*)&sessions[client_socket].active_data_addr, sizeof(sessions[client_socket].active_data_addr)) == -1)
        {
            perror("connect");
            close(data_fd);
            sendResponse("425 Can't open data connection.\r\n", client_socket);
            return;
        }
    }
    else
    {
        sendResponse("425 Can't open data connection.\r\n", client_socket);
        return;
    }

    char* buffer = (char*)calloc(config.buffer_size, sizeof(char));
    std::ofstream file(sessions[client_socket].current_directory + "/" + filename, std::ios::binary);
    if (!file)
    {
        perror("ofstream");
        free(buffer);
        close(data_fd);
        sendResponse("553 Requested action not taken. File name not allowed.\r\n", client_socket);
        return;
    }

    int bytes_read;
    while ((bytes_read = recv(data_fd, buffer, config.buffer_size, 0)) > 0)
    {
        if (sessions[client_socket].transfer_type == ASCII)
        {
            std::string data(buffer, bytes_read);
            data = std::regex_replace(data, std::regex("\r\n"), "\n");
            file.write(data.c_str(), data.size());
        }
        else
        {
            file.write(buffer, bytes_read);
        }

        if (!file)
        {
            perror("ofstream write");
            free(buffer);
            close(data_fd);
            sendResponse("426 Connection closed; transfer aborted.\r\n", client_socket);
            return;
        }
    }
    if (bytes_read < 0)
    {
        perror("recv");
        free(buffer);
        close(data_fd);
        sendResponse("426 Connection closed; transfer aborted.\r\n", client_socket);
        return;
    }

    file.close();
    free(buffer);
    close(data_fd);
    sendResponse("226 Closing data connection. Requested file action successful.\r\n", client_socket);
}

void FTPServer::handleRetrieve(const std::string& filename, int client_socket)
{
    if (!sessions[client_socket].logged_in)
    {
        sendResponse("530 Not logged in.\r\n", client_socket);
        return;
    }

    std::cout << "[TRACE]: RETR " << filename << "\n";
    sendResponse("150 File status okay; about to open data connection.\r\n", client_socket);

    int data_fd;
    if (sessions[client_socket].passive_mode)
    {
        sockaddr_in data_addr;
        socklen_t addrlen = sizeof(data_addr);
        data_fd = accept(data_socket, (struct sockaddr*)&data_addr, &addrlen);
        if (data_fd < 0)
        {
            perror("accept");
            sendResponse("425 Can't open data connection.\r\n", client_socket);
            return;
        }
    }
    else if (sessions[client_socket].active_mode)
    {
        data_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (data_fd == -1)
        {
            perror("socket");
            sendResponse("425 Can't open data connection.\r\n", client_socket);
            return;
        }
        if (connect(data_fd, (struct sockaddr*)&sessions[client_socket].active_data_addr, sizeof(sessions[client_socket].active_data_addr)) == -1)
        {
            perror("connect");
            close(data_fd);
            sendResponse("425 Can't open data connection.\r\n", client_socket);
            return;
        }
    }
    else
    {
        sendResponse("425 Can't open data connection.\r\n", client_socket);
        return;
    }

    std::ifstream file(sessions[client_socket].current_directory + "/" + filename, std::ios::binary);
    if (file)
    {
        char* buffer = (char*)calloc(config.buffer_size, sizeof(char));
        while (file)
        {
            file.read(buffer, config.buffer_size);
            std::streamsize bytes_read = file.gcount();

            if (sessions[client_socket].transfer_type == ASCII)
            {
                std::string data(buffer, bytes_read);
                data = std::regex_replace(data, std::regex("\n"), "\r\n");

                char *data_ptr = &data[0];
                std::streamsize data_size = data.size();
                while (data_size > 0)
                {
                    ssize_t bytes_sent = send(data_fd, data_ptr, data_size, 0);
                    if (bytes_sent == -1)
                    {
                        perror("send");
                        free(buffer);
                        close(data_fd);
                        sendResponse("426 Connection closed; transfer aborted.\r\n", client_socket);
                        return;
                    }
                    data_ptr += bytes_sent;
                    data_size -= bytes_sent;
                }
            }
            else
            {
                char *data_ptr = buffer;
                while (bytes_read > 0)
                {
                    ssize_t bytes_sent = send(data_fd, data_ptr, bytes_read, 0);
                    if (bytes_sent == -1)
                    {
                        perror("send");
                        free(buffer);
                        close(data_fd);
                        sendResponse("426 Connection closed; transfer aborted.\r\n", client_socket);
                        return;
                    }
                    data_ptr += bytes_sent;
                    bytes_read -= bytes_sent;
                }
            }
        }
        free(buffer);
        sendResponse("226 Closing data connection. Requested file action successful.\r\n", client_socket);
    }
    else
    {
        sendResponse("550 Requested action not taken. File unavailable.\r\n", client_socket);
    }

    close(data_fd);
}

void FTPServer::handleSystem(int client_socket)
{
    std::cout << "[TRACE]: SYST\n";
    sendResponse("215 UNIX Type: L8\r\n", client_socket);
}

void FTPServer::handleType(const std::string& type, int client_socket)
{
    if (type == "A")
    {
        sessions[client_socket].transfer_type = ASCII;
        sendResponse("200 Switching to ASCII mode.\r\n", client_socket);
    }
    else if (type == "I")
    {
        sessions[client_socket].transfer_type = BINARY;
        sendResponse("200 Switching to Binary mode.\r\n", client_socket);
    }
    else
        sendResponse("500 Unrecognized TYPE command.\r\n", client_socket);
}

void FTPServer::sendResponse(const std::string& response, int client_socket)
{
    std::cout << "[TRACE]: Sending response: " << response;
    send(client_socket, response.c_str(), response.length(), 0);
}
