// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#include <time.h>

#include "nic/sdk/platform/evutils/include/evutils.h"
#include "nic/sdk/platform/ncsi/ncsi_mgr.h"
#include "nic/sdk/lib/logger/logger.hpp"
#include "delphi_ipc.h"
#include "grpc_ipc.h"

#define ARRAY_LEN(var)   (int)((sizeof(var)/sizeof(var[0])))

#define NCSI_TRACE_ERR(fmt...)             \
    if (GetCurrentLogger()) {              \
        GetCurrentLogger()->error(fmt);    \
    }

#define NCSI_TRACE_INFO(fmt...)            \
    if (GetCurrentLogger()) {              \
        GetCurrentLogger()->info(fmt);     \
    }

typedef std::shared_ptr<spdlog::logger> Logger;
static Logger current_logger;
using namespace std;
using namespace sdk::platform::ncsi;

int usage(int argc, char* argv[]);
void ipc_init();

Logger CreateLogger(const char* log_name) {

    char log_path[64] = {0};
    snprintf(log_path, sizeof(log_path), "/var/log/pensando/%s", log_name);

    auto rotating_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>
        (log_path, 1*1024*1024, 3);

    Logger _logger = std::make_shared<spdlog::logger>(log_name, rotating_sink);

    _logger->set_pattern("[%Y-%m-%d_%H:%M:%S.%e] %L %v");
    _logger->flush_on(spdlog::level::info);

#ifdef DEBUG_ENABLE
    spdlog::set_level(spdlog::level::debug);
#endif

    return _logger;
}

void SetCurrentLogger(Logger logger) {
    current_logger = logger;

    return;
}

Logger GetCurrentLogger() {
    return current_logger;
}

int ncsi_logger (sdk_trace_level_e trace_level, const char *format, ...)
{
    char       logbuf[1024];
    va_list    args;

    va_start(args, format);
    vsnprintf(logbuf, sizeof(logbuf), format, args);
    va_end(args);

    switch (trace_level) {
    case sdk::lib::SDK_TRACE_LEVEL_ERR:
        NCSI_TRACE_ERR("{}", logbuf);
        break;
    case sdk::lib::SDK_TRACE_LEVEL_INFO:
        NCSI_TRACE_INFO("{}", logbuf);
        break;
    default:
        break;
    }
    return 0;
}
shared_ptr<DelphiIpcService> ncsid_delphi_svc;
shared_ptr<grpc_ipc> grpc_ipc_svc;

delphi::SdkPtr g_sdk;
NcsiMgr *ncsimgr;
std::string transport_mode = "RBT";
transport* xport_obj;
char iface_name[32] = "oob_mnic0";

bool is_interface_online(const char* interface) {
    struct ifreq ifr;
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    memset(&ifr, 0, sizeof(ifr));
    strcpy(ifr.ifr_name, interface);
    if (ioctl(sock, SIOCGIFFLAGS, &ifr) < 0) {
        //perror("SIOCGIFFLAGS");
    }
    close(sock);
    return !!(ifr.ifr_flags & IFF_UP);
}

void delphi_mount_cb()
{
    Logger logger_obj;

    //Create the logger
    logger_obj = CreateLogger("ncsi.log");
    SetCurrentLogger(logger_obj);

    sdk::lib::logger::init(ncsi_logger);

    if (!(transport_mode.compare("RBT")))
        xport_obj = new rbt_transport(iface_name);
    else if (transport_mode.compare("MCTP"))
        xport_obj = new mctp_transport();
    else {
        printf("Illegal transport mode provided.");
        return;
    }

    printf("Initializing ncsi transport in %s mode\n", transport_mode.c_str());
    while(!is_interface_online(iface_name)) {
        //printf("Interface is not online, waiting...\n");
        usleep(100000);
    }

    ncsimgr->Init(xport_obj, grpc_ipc_svc);

    return;
}

void delphi_init()
{
    delphi::SdkPtr sdk(make_shared<delphi::Sdk>());
    g_sdk = sdk;
    ncsid_delphi_svc = make_shared<DelphiIpcService>(sdk, delphi_mount_cb);

    ncsid_delphi_svc->Init(ncsid_delphi_svc);

    return;
}

int main(int argc, char* argv[])
{
   ncsimgr = new NcsiMgr();
   grpc_ipc_svc = make_shared<grpc_ipc>();

    if (argc > 1) {
        if (!strcmp(argv[1], "RBT"))
            printf("trasnport mode is RBT\n");
        else if (!strcmp(argv[1], "MCTP"))
            printf("trasnport mode is MCTP\n");
        else
        {
            printf("Invalid transport mode\n");
            return usage(argc, argv);
        }
    }
    if (argc > 2)
        strncpy(iface_name, argv[2], sizeof(iface_name));

    grpc_ipc_svc->connect_hal();
    delphi_init();

    printf("Waiting for onmountcomplete...\n");
    evutil_run(EV_DEFAULT);

    //Should never reach here
    printf("should never be printed !!!\n");

    return 0;
}

int usage(int argc, char* argv[])
{
    printf("Usage: %s [transport mode]\n",argv[0]);
    printf("Possible transport modes are:\n");
    printf("   %-16s\n","RBT (default)");
    printf("   %-16s\n","MCTP");

    return 1;
}

