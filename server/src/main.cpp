#include "ftp/server.hpp"

int main()
{
    FTPServer server(FTPServerConfig{});
    server.start();
    return EXIT_SUCCESS;
}
