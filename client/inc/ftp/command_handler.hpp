#pragma once
#include "connection.hpp"
#include "data_transfer.hpp"
#include <string>

namespace FTP_Client {

class CommandHandler {
public:
    CommandHandler(Connection* connection);
    bool login();
    bool process_command(const std::string& command_line);

private:
    Connection* connection;

    bool handle_ls();
    bool handle_cd(const std::string& args);
    bool handle_pwd();
    bool handle_get(const std::string& filename);
    bool handle_put(const std::string& filename);
    bool parse_pasv_response(const std::string& response, std::string& ip, int& port);
};

} // namespace FTP_Client
