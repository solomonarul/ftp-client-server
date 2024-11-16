#include "ftp/pi.hpp"

using namespace FTP_Server;

/* #region Setter. */

void PI::set_dtp(DTP* process)
{
    this->process = process;
}

void PI::set_port(uint16_t port)
{
    this->port = port;
}

/* #endregion */
/* #region Getters. */

uint16_t PI::get_port()
{
    return this->port;
}

/* #endregion */