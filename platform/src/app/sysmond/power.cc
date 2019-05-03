/*
 * Copyright (c) 2019, Pensando Systems Inc.
 */

#include "sysmond.h"
#include "nic/sdk/platform/sensor/sensor.hpp"

static delphi::objects::asicpowermetrics_t    asicpower;
using namespace sdk::platform::sensor;

static void
checkpower(void)
{
    uint64_t key = 0;
    int ret;
    static int max_pin;
    static int max_pout1;
    static int max_pout2;
    sdk::platform::sensor::system_power_t power;

    ret = sdk::platform::sensor::read_powers(&power);
    if (!ret) {
        power.pin /= 1000;
        if (max_pin < power.pin) {
            TRACE_INFO(GetLogger(), "Power of {:s} is : {:d}mW",
                       "pin", power.pin);
            max_pin = power.pin;
        }
        asicpower.pin = power.pin;

        power.pout1 /= 1000;
        if (max_pout1 < power.pout1) {
            TRACE_INFO(GetLogger(), "Power of {:s} is : {:d}mW",
                       "pout1", power.pout1);
            max_pout1 = power.pout1;
        }
        asicpower.pout1 = power.pout1;

        power.pout2 /= 1000;
        if (max_pout2 < power.pout2) {
            TRACE_INFO(GetLogger(), "Power of {:s} is : {:d}mW",
                       "pout2", power.pout2);
            max_pout2 = power.pout2;
        }
        asicpower.pout2 = power.pout2;

        //Publish Delphi object
        delphi::objects::AsicPowerMetrics::Publish(key, &asicpower);
    } else {
        TRACE_ERR(GetLogger(), "Reading power failed");
    }
    return;
}

MONFUNC(checkpower);
