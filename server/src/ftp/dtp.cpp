#include "ftp/dtp.hpp"

using namespace FTP_Server;

/* #region Setters */

void DTP::set_pi(PI* interpreter)
{
    this->interpreter = interpreter;
}

void DTP::set_port(uint16_t port)
{
    this->port = port;
}

/* #endregion */