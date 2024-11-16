#include "ftp/instance.hpp"
#include <thread>

using namespace std::chrono;
using namespace FTP_Server;

Instance::Instance(InstanceConfig config)
{
    // Establish communications between components.
    this->interpreter.set_dtp(&this->process);
    this->process.set_pi(&this->interpreter);

    // Setup defaults.
    this->interpreter.set_port(config.control_port);
    this->process.set_port(config.data_port);
}

void Instance::start_listening()
{

}

void Instance::wait()
{
    while(running) std::this_thread::sleep_for(1ms);
}

uint16_t Instance::get_pi_port()
{
    return this->interpreter.get_port();
}