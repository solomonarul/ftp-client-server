#pragma once

#include <cstdint>
#include "dtp.hpp"
#include "pi.hpp"

namespace FTP_Server
{
    struct InstanceConfig
    {
        uint16_t control_port = 21;
        uint16_t data_port = 20;
    };

    class Instance
    {
    private:
        DTP process;
        PI interpreter;

        bool running = true;    // TODO: replace this with a condition variable.
    public:
        Instance(InstanceConfig config);
        void start_listening();
        void wait();

        uint16_t get_pi_port();
    };
};