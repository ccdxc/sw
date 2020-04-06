/*
 * Copyright (c) 2019, Pensando Systems Inc.
 */

#include "sysmon_internal.hpp"
#include "platform/sensor/sensor.hpp"

using namespace sdk::platform::sensor;

#define INCREASE_THRESHOLD 1.1

void
checkpower(void)
{
    int ret;
    static int max_pin;
    static int max_pout1;
    static int max_pout2;
    sdk::platform::sensor::system_power_t power;

    memset(&power, 0, sizeof(sdk::platform::sensor::system_power_t));

    ret = sdk::platform::sensor::read_powers(&power);
    if (ret == SDK_RET_OK) {
        power.pin /= 1000;
        if (max_pin < power.pin) {
            if (power.pin > (power.pin * INCREASE_THRESHOLD)) {
                SDK_HMON_TRACE_INFO("Power of %s is %umW",
                                    "pin", power.pin);
            }
            max_pin = power.pin;
        }

        power.pout1 /= 1000;
        if (max_pout1 < power.pout1) {
            if (power.pout1 > (power.pout1 * INCREASE_THRESHOLD)) {
                SDK_HMON_TRACE_INFO("Power of %s is %umW",
                                    "pout1", power.pout1);
            }
            max_pout1 = power.pout1;
        }

        power.pout2 /= 1000;
        if (max_pout2 < power.pout2) {
            if (power.pout2 > (power.pout2 * INCREASE_THRESHOLD)) {
                SDK_HMON_TRACE_INFO("Power of %s is %umW",
                                    "pout2", power.pout2);
            }
            max_pout2 = power.pout2;
        }
        if (g_sysmon_cfg.power_event_cb) {
            g_sysmon_cfg.power_event_cb(&power);
        }
    } else {
        SDK_HMON_TRACE_ERR("Reading power failed");
    }
    return;
}

// MONFUNC(checkpower);
