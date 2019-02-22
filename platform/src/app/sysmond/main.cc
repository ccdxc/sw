/*
 * Copyright (c) 2019, Pensando Systems Inc.
 */
#include "sysmond.h"
#include "sysmond_delphi.hpp"

::utils::log *g_trace_logger;

extern monfunc_t __start_monfunclist[];
extern monfunc_t __stop_monfunclist[];

static void monitorsystem() {
    for (monfunc_t *c = __start_monfunclist; c < __stop_monfunclist; c++) {
        c->func();
    }
    return;
}

void initializeLogger() {
    static bool initDone = false;

    if (!initDone) {
        g_trace_logger = ::utils::log::factory("sysmond", 0x0,
                                        ::utils::log_mode_sync, false,
                                        LOG_FILENAME, LOG_MAX_FILESIZE,
                                        LOG_MAX_FILES, ::utils::trace_debug,
                                        ::utils::log_none);

        initDone = true;
    }
}

int
main(int argc, char *argv[])
{
    //initialize the logger
    initializeLogger();

    TRACE_INFO(GetLogger(), "Monitoring system events");
    // initialize the pal
#ifdef __x86_64__
    assert(sdk::lib::pal_init(platform_type_t::PLATFORM_TYPE_SIM) == sdk::lib::PAL_RET_OK);
#elif __aarch64__
    assert(sdk::lib::pal_init(platform_type_t::PLATFORM_TYPE_HAPS) == sdk::lib::PAL_RET_OK);
#endif

    // Register for mac metrics
    delphi::objects::AsicTemperatureMetrics::CreateTable();
    delphi::objects::AsicPowerMetrics::CreateTable();

    delphi::SdkPtr sdk(make_shared<delphi::Sdk>());
    shared_ptr<SysmondService> svc = make_shared<SysmondService>(sdk, "Sysmond");
    svc->init();
    sdk->RegisterService(svc);

    pthread_t delphi_thread;
    pthread_create(&delphi_thread, NULL, delphi_thread_run, reinterpret_cast<void *>(&sdk));

    //Set LED to green.
    pal_system_set_led(LED_COLOR_GREEN, LED_FREQUENCY_0HZ);

    while (1) {
        // Dont block context switches, let the process sleep for some time
        sleep(10);
        // Poll the system variables
        monitorsystem();
        TRACE_FLUSH(GetLogger());
    }
    return 0;
}
