//------------------------------------------------------------------------------
// starting point for network agent bootup
//------------------------------------------------------------------------------

#include <stdlib.h>
#include <iostream>
#include <signal.h>
#include <memory>
#include <string>
#include <grpc++/grpc++.h>
#include "base.h"
#include "../hal/hal.hpp"
#include "net.grpc.pb.h"
#include "svc_reg.hpp"

// TODO: this should come from some cfg file ideally
//       cmd line argument
const std::string&    nwAgentEndpoint("localhost:50051");

#define INSTALL_SIGNAL_HANDLER(sig, handler)                             \
{                                                                        \
    if (signal((sig), (handler)) == SIG_ERR) {                           \
        std::cout << "Failed to install signal "                         \
                  << (sig) << " handler" << std::endl;                   \
        return -1;                                                       \
    }                                                                    \
}

static void
handle_signals (int sig)
{
    std::cout << "Rcvd signal " << sig << std::endl;

    switch (sig) {
    case SIGINT:
    case SIGUSR1:
    case SIGUSR2:
    default:
        exit(sig);
        break;
    }
}

// TODO: need to figure a clean way for HAL to exit
// 1. hook on to D-Bus esp. system bus
// 2. handle signals
// 3. need to release all memory/resources and kill the FTE threads etc.
int
main (int argc, char **argv)
{
    hal::hal_cfg_t    hal_cfg = { 0 };
    std::cout << "Starting Network Agent ..." << std::endl;

    // install all the signal handlers
    INSTALL_SIGNAL_HANDLER(SIGINT, handle_signals);
    INSTALL_SIGNAL_HANDLER(SIGUSR1, handle_signals);
    INSTALL_SIGNAL_HANDLER(SIGUSR2, handle_signals);

    // spawn internal threads

    hal::hal_parse_cfg(std::string("/tmp/cfg.json").c_str(), &hal_cfg);
    // initialize HAL
    hal::hal_init(&hal_cfg);

    // finally register all our gRPC services and start listening
    RegistergRPCServers(nwAgentEndpoint);

    // wait for HAL to finish its job(s)/thread(s)
    hal::hal_wait();
}
