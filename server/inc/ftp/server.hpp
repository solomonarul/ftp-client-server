#pragma once

#include <string>
#include <netinet/in.h>
#include <unistd.h>
#include <dirent.h>
#include <unordered_map>

#include <cstring>
#include <arpa/inet.h>

struct FTPServerConfig
{
    int port = 21;
    int buffer_size = 1024;
    std::string root_dir;
    std::string user = "admin";
    std::string pass = "admin";
};

enum TransferType { ASCII, BINARY };

struct Session
{
    bool logged_in = false;
    bool passive_mode = false;
    bool active_mode = false;
    TransferType transfer_type = BINARY;
    std::string current_directory = ".";
    sockaddr_in active_data_addr;
};

class FTPServer
{
public:
    FTPServer(FTPServerConfig config);
    ~FTPServer();
    void start();

private:
    int server_fd;
    struct sockaddr_in address;
    int data_socket;
    int client_socket;
    FTPServerConfig config;
    std::unordered_map<int, Session> sessions;

    void handleClient(int client_socket);
    void handleCommand(const std::string& command, int client_socket);
    void handleUser(const std::string& user, int client_socket);
    void handlePassword(const std::string& pass, int client_socket);
    void handleList(int client_socket);
    void handlePWD();
    void handleStore(const std::string& filename, int client_socket);
    void handleRetrieve(const std::string& filename, int client_socket);
    void handlePort(const std::string& params, int client_socket);
    void handleSystem(int client_socket);
    void handleType(const std::string& type, int client_socket);
    void sendResponse(const std::string& response, int client_socket);
    void handleCwd(const std::string& directory, int client_socket);
    void handlePassive();
};