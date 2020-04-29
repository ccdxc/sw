// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#include <time.h>

#include "nic/sdk/platform/evutils/include/evutils.h"
#include "nic/sdk/platform/ncsi/ncsi_mgr.h"
#include "nic/sdk/platform/pal/include/pal.h"
#include "nic/sdk/lib/logger/logger.hpp"
#include "grpc_ipc.h"

#define ALOM_PRESENT           0x20

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

int ncsi_logger (uint32_t mod_id, sdk_trace_level_e trace_level,
                 const char *format, ...)
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
shared_ptr<grpc_ipc> grpc_ipc_svc;

NcsiMgr *ncsimgr;
std::string transport_mode = "RBT";
transport* xport_obj;
char iface_name[32] = "oob_mnic0";

bool is_interface_online(const char* interface) {
    struct ifreq ifr;
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);

    if (sock == -1) {
        perror("SOCKET OPEN");
        return false;
    }

    memset(&ifr, 0, sizeof(ifr));
    strcpy(ifr.ifr_name, interface);

    ifr.ifr_flags = (IFF_UP | IFF_BROADCAST | IFF_RUNNING | IFF_MULTICAST);

    if (ioctl(sock, SIOCSIFFLAGS, &ifr) < 0) {
        perror("SIOCSIFFLAGS");
        close(sock);
        return false;
    }
    
    memset(&ifr, 0, sizeof(ifr));
    strcpy(ifr.ifr_name, interface);
    if (ioctl(sock, SIOCGIFFLAGS, &ifr) < 0) {
        perror("SIOCGIFFLAGS");
        close(sock);
        return false;
    }

    close(sock);
    return !!(ifr.ifr_flags & IFF_UP);
}

void InitNcsiMgr()
{
    if (!(transport_mode.compare("RBT")))
        xport_obj = new rbt_transport(iface_name);
    else if (transport_mode.compare("MCTP"))
        xport_obj = new mctp_transport();
    else {
        SDK_TRACE_INFO("Invalid transport mode: %s", transport_mode.c_str());
        return;
    }

    SDK_TRACE_INFO("Initializing ncsi transport in %s mode", 
            transport_mode.c_str());
    while(!is_interface_online(iface_name)) {
        //SDK_TRACE_INFO("Interface is not online, waiting...\n");
        usleep(10000);
    }

    SDK_TRACE_INFO("NCSI interface is UP !"); 
    ncsimgr->Init(xport_obj, grpc_ipc_svc);

    return;
}

int main(int argc, char* argv[])
{
    int32_t cpld_cntl_reg;
    ncsimgr = new NcsiMgr();
    grpc_ipc_svc = make_shared<grpc_ipc>();
    Logger logger_obj;

    //Create the logger
    logger_obj = CreateLogger("ncsi.log");
    SetCurrentLogger(logger_obj);

    sdk::lib::logger::init(ncsi_logger);

    if (argc > 1) {
        if (!strcmp(argv[1], "RBT"))
            SDK_TRACE_INFO("transport mode: RBT");
        else if (!strcmp(argv[1], "MCTP"))
            SDK_TRACE_INFO("transport mode: MCTP");
        else
        {
            SDK_TRACE_INFO("Invalid transport mode");
            return usage(argc, argv);
        }
    }
    if (argc > 2)
        strncpy(iface_name, argv[2], sizeof(iface_name));

    /* If listening on oob interface, then we must be connected to ALOM 
     * in order to receive NCSI packet from BMC */
    if (!strcmp(iface_name, "oob_mnic0")) {
        cpld_cntl_reg = cpld_reg_rd(CPLD_REGISTER_CTRL);
        if (cpld_cntl_reg == -1) {
            SDK_TRACE_INFO("Error reading CPLD control reg for ALOM presence."
                    "Exiting ncsid app !");
            return 0;
        }
        if (! (cpld_cntl_reg & ALOM_PRESENT)) {
            SDK_TRACE_INFO("ALOM is not present. NCSI cannot function without ALOM."
                    "Exiting ncsid app !");
            return 0;
        }
    }

    grpc_ipc_svc->connect_hal();
    InitNcsiMgr();
    evutil_run(EV_DEFAULT);

    //Should never reach here
    SDK_TRACE_INFO("should never reach here. Exiting ncsid app !");

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

