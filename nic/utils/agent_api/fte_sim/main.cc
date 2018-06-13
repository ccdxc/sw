#include <iostream>
#include "nic/utils/agent_api/agent_api.hpp"
#include "nic/utils/agent_api/shared_constants.h"
#include "nic/gen/proto/hal/fwlogsim.grpc.pb.h"
#include "nic/utils/agent_api/fte_sim/fwlogsim_svc.hpp"
using namespace std;
using grpc::ServerBuilder;
using fwlog::FWEvent;
ipc_logger *iLogger[3];
std::mutex mtx;           // mutex for critical section

void run_server(std::string &addr) {
    FWLogSimServiceImpl service{};

    ServerBuilder builder;
    builder.AddListeningPort(addr, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cout << "Server listening on " << addr << std::endl;
    server->Wait();
}

int main(int argc, char **argv) {
    int ix;

    std::string server_addr("0.0.0.0:");
    if (argc > 1) {
        server_addr.append(argv[1]);
    } else {
        server_addr.append("14990");
    }

    int ret = ipc_logger::init();
    if (ret != 0) {
        std::cout << "Error initializing ipc";
        exit(ret);
    }

    for (ix = 0; ix < IPC_INSTANCES; ix++) {
    iLogger[ix] = ipc_logger::factory();
        if (iLogger[ix] == NULL) {
            std::cout << "Error creating ipc";
            exit(-1);
        }
    }

    std::cout << "Starting fwlogsim.." << server_addr << std::endl;
    run_server(server_addr);
}

int channel = 0;
void log_firew_event(const FWEvent& fwe)
{
    std::cout << "log_firew_event.." << fwe.fwaction() << std::endl;
    mtx.lock();
    channel = (channel + 1) % IPC_INSTANCES;
    iLogger[channel]->fw_log(fwe);
    mtx.unlock();
}
