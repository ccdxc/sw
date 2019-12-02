/*
 * Copyright (c) 2019, Pensando Systems Inc.
 */
#include "sysmon_internal.hpp"

sysmon_cfg_t g_sysmon_cfg;

systemled_t currentstatus = {UKNOWN_STATE, LED_COLOR_NONE};

monfunc_t monfunclist[] = {
    { checkcattrip     },
    { checkfrequency   },
    { checkruntime     },
    { checktemperature },
    { checkdisk        },
    { checkmemory      },
    { checkpostdiag    },
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
sysmon_init (sysmon_cfg_t *sysmon_cfg)
{
    systemled_t led;

    if (sysmon_cfg == NULL) {
        SDK_TRACE_ERR("Invalid params, cfg is NULL");
        return -1;
    }

    g_sysmon_cfg = *sysmon_cfg;

    SDK_TRACE_INFO("Monitoring system events");

    //check for panic dump
    checkpanicdump();

    SDK_TRACE_INFO("HBM Threshold temperature is %u",
                   g_sysmon_cfg.catalog->hbmtemperature_threshold());

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
sysmon_monitor (void)
{
    // Poll the system variables
    monitorsystem();
    return 0;
}
