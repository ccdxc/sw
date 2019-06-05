/*
 * Copyright (c) 2019, Pensando Systems Inc.
 */
#include "sysmond.h"
#include "nic/sdk/platform/capri/csrint/csr_init.hpp"

::utils::log *g_trace_logger;
::utils::log *obfl_trace_logger;
extern sdk::lib::catalog *catalog;
extern int configurefrequency();

extern monfunc_t __start_monfunclist[];
extern monfunc_t __stop_monfunclist[];

shared_ptr<SysmondService> svc;
systemled_t currentstatus = {UKNOWN_STATE, LED_COLOR_NONE};

static void monitorsystem() {
    for (monfunc_t *c = __start_monfunclist; c < __stop_monfunclist; c++) {
        c->func();
    }
    return;
}

void
sysmgrsystemled (systemled_t led) {

    //Check if already at max warning level.
    if (led.event >= currentstatus.event) {
        return;
    }
    switch (led.event) {
        case CRITICAL_EVENT:
            currentstatus.event = CRITICAL_EVENT;
            currentstatus.color = LED_COLOR_YELLOW;
            pal_system_set_led(LED_COLOR_YELLOW, LED_FREQUENCY_0HZ);
            break;
        case NON_CRITICAL_EVENT:
            currentstatus.event = NON_CRITICAL_EVENT;
            currentstatus.color = LED_COLOR_YELLOW;
            pal_system_set_led(LED_COLOR_YELLOW, LED_FREQUENCY_0HZ);
            break;
        case PROCESS_CRASHED_EVENT:
            currentstatus.event = PROCESS_CRASHED_EVENT;
            currentstatus.color = LED_COLOR_YELLOW;
            pal_system_set_led(LED_COLOR_YELLOW, LED_FREQUENCY_0HZ);
            break;
        case EVERYTHING_WORKING:
            currentstatus.event = EVERYTHING_WORKING;
            currentstatus.color = LED_COLOR_GREEN;
            pal_system_set_led(LED_COLOR_GREEN, LED_FREQUENCY_0HZ);
            break;
        default:
            return;
    }
    return;
}

void initializeLogger() {
    static bool initDone = false;
    LogMsg::Instance().get()->setMaxErrCount(0);
    if (!initDone) {
        g_trace_logger = ::utils::log::factory("sysmond", 0x0,
                                        ::utils::log_mode_sync, false,
                                        LOG_FILENAME, LOG_MAX_FILESIZE,
                                        LOG_MAX_FILES, ::utils::trace_debug,
                                        ::utils::log_none);
        obfl_trace_logger = ::utils::log::factory("sysmond_obfl", 0x0,
                                        ::utils::log_mode_sync, false,
                                        OBFL_LOG_FILENAME, OBFL_LOG_MAX_FILESIZE,
                                        LOG_MAX_FILES, ::utils::trace_debug,
                                        ::utils::log_none);
        initDone = true;
    }
}

int
main(int argc, char *argv[])
{
    systemled_t led;
    //initialize the logger
    initializeLogger();
    sdk::lib::logger::init(local_sdk_logger, obfl_sdk_logger);

    TRACE_INFO(GetLogger(), "Monitoring system events");
    TRACE_INFO(GetObflLogger(), "Monitoring system memory");

    // initialize the pal
#ifdef __x86_64__
    assert(sdk::lib::pal_init(platform_type_t::PLATFORM_TYPE_SIM) == sdk::lib::PAL_RET_OK);
#elif __aarch64__
    assert(sdk::lib::pal_init(platform_type_t::PLATFORM_TYPE_HAPS) == sdk::lib::PAL_RET_OK);
#endif

    //check for panic dump
    checkpanicdump();

    // Register for sysmond metrics
    delphi::objects::AsicTemperatureMetrics::CreateTable();
    delphi::objects::AsicPowerMetrics::CreateTable();
    delphi::objects::AsicFrequencyMetrics::CreateTable();
    delphi::objects::AsicMemoryMetrics::CreateTable();

    sdk::platform::capri::csr_init();
    catalog = sdk::lib::catalog::factory();
    TRACE_INFO(GetLogger(), "HBM Threshold temperature is {}", catalog->hbmtemperature_threshold());

    if (configurefrequency() == 0) {
        TRACE_INFO(GetLogger(), "Frequency set from file");
    } else {
        TRACE_INFO(GetLogger(), "Failed to set frequency from file");
    }
    delphi::SdkPtr sdk(make_shared<delphi::Sdk>());
    svc = make_shared<SysmondService>(sdk, "Sysmond");
    svc->init();
    sdk->RegisterService(svc);

    pthread_t delphi_thread;
    pthread_create(&delphi_thread, NULL, delphi_thread_run, reinterpret_cast<void *>(&sdk));

    led.event = EVERYTHING_WORKING;
    sysmgrsystemled(led);

    while (1) {
        // Dont block context switches, let the process sleep for some time
        sleep(10);
        // Poll the system variables
        monitorsystem();
        TRACE_FLUSH(GetLogger());
        TRACE_FLUSH(GetObflLogger());
    }
    return 0;
}
