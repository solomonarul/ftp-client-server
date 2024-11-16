#include "ftp/instance.hpp"
#include <cstdlib>
#include <iostream>

using namespace FTP_Server;

int main(int argc, char* argv[])
{
    Instance server(InstanceConfig{});
    server.start_listening();
    std::cout << "[SRVR]: Started listening on port " << server.get_pi_port() << ".\n";
    server.wait();
    return EXIT_SUCCESS;
}