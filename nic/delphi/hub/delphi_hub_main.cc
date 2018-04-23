// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <ev++.h>

#include "delphi_server.hpp"

using namespace std;
using namespace delphi;

int main(int argc, char **argv) {
    ev::default_loop loop;

    // instantiate the delphi server
    DelphiServerPtr server(make_shared<DelphiServer>());
    server->Start();
    LogInfo("Delphi server is running.");

    // enter the main event loop
    loop.run(0);
}
