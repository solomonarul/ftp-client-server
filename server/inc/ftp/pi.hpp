#pragma once

#include <cstdint>

namespace FTP_Server
{
    class DTP;

    class PI
    {
    private:
        DTP* process = nullptr;
        uint16_t port = -1;

    public:
        void set_dtp(DTP* process);
        void set_port(uint16_t port);
        uint16_t get_port();
    };
};
