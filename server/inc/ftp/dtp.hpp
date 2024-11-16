#pragma once

#include <cstdint>

namespace FTP_Server
{
    class PI;

    class DTP
    {
    private:
        PI* interpreter = nullptr;
        uint16_t port = -1;

    public:
        void set_pi(PI* interpreter);
        void set_port(uint16_t port);
    };
};