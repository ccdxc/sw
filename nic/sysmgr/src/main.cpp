// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <memory>
#include <string>

#include <ev++.h>
#include <stdlib.h>
#include <spdlog/sinks/null_sink.h>

#include "bus_api.hpp"
#include "bus_callbacks.hpp"
#include "cgroups.hpp"
#include "events_api.hpp"
#include "log.hpp"
#include "service_factory.hpp"
#include "utils.hpp"
#include "vmstats.hpp"
#include "watchdog.hpp"

SysmgrBusPtr g_bus;
SysmgrEventsPtr g_events;

int
main (int argc, char *argv[])
{
    ev::default_loop loop;
    ServiceFactoryPtr service_factory = ServiceFactory::getInstance();
    WatchdogPtr watchdog;
    
    cg_init();
    vmstats_init(loop);

    if (argc == 3) {
        log_location = argv[2];
    }

    mkdirs(log_location.c_str());
   
    int rc = setpgid(0, 0);
    if (rc == -1) 
    {
        perror("setpgid");
        exit(-1);
    }
    redirect_stds("sysmgr", getpid());
    if (getenv("SYSMGR_CORES") != NULL) {
        unsigned long cores = std::stoul(getenv("SYSMGR_CORES"), nullptr, 16);
        cpulock(cores);
        g_log->info("Restricting sysmgr to 0x%lx cores", cores);
    } else {
        cpulock(0xffffffff);
    }

    g_bus = init_bus(&g_bus_callbacks);
    g_events = init_events(spdlog::create<spdlog::sinks::null_sink_st>("null_logger"));
    
    watchdog = Watchdog::create();
    
    if (argc > 1)
    {
        service_factory->load_config(argv[1]);
    }
    else
    {
        std::string config_file;
        try {
            config_file = get_main_config_file();
        } catch (const std::exception & e) {
            g_log->err("get_main_config_file() exception: %s",
                       std::string(e.what()).c_str());
            exit(-1);
        }
        service_factory->load_config(config_file);
    }
    
    g_log->debug("Trying /data/sysmgr.json");
    service_factory->load_config("/data/sysmgr.json");
    g_log->debug("Done with /data/sysmgr.json");

    // Needed in order to process custom events before blocking for an
    // actual event
    loop.run(EVRUN_NOWAIT);
    g_bus->Connect();
    loop.run(0);
    
    return 0;
}
