// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <ev++.h>
#include <string>

#include "delphi_server.hpp"

using namespace std;
using namespace delphi;

int main(int argc, char **argv) {
    ev::default_loop loop;
    const char *db_filename;

    if (argc >= 2) {
	db_filename = argv[1];
    } else {
	db_filename = "/data/delphi.dat";
    }
    
    // instantiate the delphi server
    DelphiServerPtr server(make_shared<DelphiServer>(db_filename));
    server->enableTrace();
    server->Start();
    LogInfo("Delphi server is running.");

    // enter the main event loop
    loop.run(0);
}
