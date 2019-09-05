/*
 * Copyright (c) 2019, Pensando Systems Inc.
 */
#include "sysmon.h"
#include "platform/capri/csrint/csr_init.hpp"

systemled_t currentstatus = {UKNOWN_STATE, LED_COLOR_NONE};

monfunc_t monfunclist[] = {
    { checkcattrip     },
    { checkfrequency   },
    { checkruntime     },
    { checktemperature },
    { checkdisk        },
    { checkmemory      },
    { checkpower       }
};

static void monitorsystem() {
    for (int i = 0; i < SDK_ARRAY_SIZE(monfunclist); i++) {
        monfunclist[i].func();
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

int
sysmond_init (void)
{
    systemled_t led;

    SDK_TRACE_INFO("Monitoring system events");

    // initialize the pal
#ifdef __x86_64__
    assert(sdk::lib::pal_init(platform_type_t::PLATFORM_TYPE_SIM) == sdk::lib::PAL_RET_OK);
#elif __aarch64__
    assert(sdk::lib::pal_init(platform_type_t::PLATFORM_TYPE_HW) == sdk::lib::PAL_RET_OK);
#endif

    //check for panic dump
    checkpanicdump();

    event_cb_init();

    sdk::platform::capri::csr_init();
    catalog = sdk::lib::catalog::factory();
    SDK_TRACE_INFO("HBM Threshold temperature is %u", catalog->hbmtemperature_threshold());

    if (configurefrequency() == 0) {
        SDK_TRACE_INFO("Frequency set from file");
    } else {
        SDK_TRACE_INFO("Failed to set frequency from file");
    }
    led.event = EVERYTHING_WORKING;
    sysmgrsystemled(led);

    return 0;
}

int
sysmond_monitor (void)
{
    // Poll the system variables
    monitorsystem();
    return 0;
}
