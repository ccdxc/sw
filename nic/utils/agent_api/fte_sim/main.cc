#include <iostream>
#include "nic/utils/agent_api/agent_api.hpp"
#include "nic/utils/agent_api/constants.h"
#include "nic/utils/ipc/constants.h"
#include "gen/proto/internal.grpc.pb.h"
#include "nic/utils/agent_api/fte_sim/fwlogsim_svc.hpp"
using namespace std;
using grpc::ServerBuilder;
using fwlog::FWEvent;
ipc_logger *iLogger[3];
std::mutex mtx;           // mutex for critical section

#define LOG_SIZE(ev) ev.ByteSizeLong()
#define TYPE_TO_LG_SZ(type, sz_) {                                    \
    if (type == IPC_LOG_TYPE_FW) {                                    \
       fwlog::FWEvent ev; sz_ = (LOG_SIZE(ev) + IPC_HDR_SIZE);        \
    } else {                                                          \
       sz_ = IPC_BUF_SIZE;                                            \
    }                                                                 \
}

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
    Logger logger;

    logger = spdlog::stdout_color_mt("fte_sim");
    logger->set_pattern("%L [%Y-%m-%d %H:%M:%S.%f] %P/%n: %v");
    std::string server_addr("0.0.0.0:");
    if (argc > 1) {
        server_addr.append(argv[1]);
    } else {
        server_addr.append("14990");
    }

    int ret = ipc_logger::init(logger);
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
    spdlog::drop("fte_sim");
}

int channel = 0;
void log_firew_event(const FWEvent& fwe)
{
    std::cout << "log_firew_event.." << fwe.fwaction() << std::endl;
    mtx.lock();
    channel = (channel + 1) % IPC_INSTANCES;
    uint8_t *buf = iLogger[channel]->get_buffer(LOG_SIZE(fwe));
    if (buf == NULL) {
        return;
    }

    if (!fwe.SerializeToArray(buf, LOG_SIZE(fwe))) {
        return;
    }

    int size = fwe.ByteSizeLong();
    iLogger[channel]->write_buffer(buf, size);

    mtx.unlock();
}
