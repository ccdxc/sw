// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <memory>
#include <string>

#include <ev++.h>

#include "nic/delphi/sdk/delphi_sdk.hpp"
#include "nic/utils/penlog/lib/penlog.hpp"
#include "nic/utils/penlog/lib/null_logger.hpp"

#include "delphi_service.hpp"
#include "service_factory.hpp"
#include "utils.hpp"
#include "watchdog.hpp"

delphi::SdkPtr delphi_sdk = std::make_shared<delphi::Sdk>();

int
main(int argc, char *argv[])
{
    ev::default_loop loop;
    ServiceFactoryPtr service_factory = ServiceFactory::getInstance();
    DelphiServicePtr svc = DelphiService::create(delphi_sdk);
    WatchdogPtr watchdog;

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
    cpulock(0xffffffff);

    delphi_sdk->RegisterService(svc);
    logger = penlog::logger_init(delphi_sdk, "sysmgr");
    watchdog = Watchdog::create();

    if (argc > 1)
    {
        service_factory->load_config(argv[1]);
    }
    else
    {
        service_factory->load_config(get_main_config_file());
    }
    
    logger->debug("Trying /data/sysmgr.json");
    service_factory->load_config("/data/sysmgr.json");
    logger->debug("Done with /data/sysmgr.json");

    // Needed in order to process custom events before blocking for an
    // actual event
    loop.run(EVRUN_NOWAIT);
    delphi_sdk->Connect();
    loop.run(0);
    
    return 0;
}
