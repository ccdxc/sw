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
    uint8_t key = 0;
    uint32_t ret;
    sdk::platform::sensor::system_power_t power;

    ret = sdk::platform::sensor::read_powers(&power);
    if (!ret) {
        asicpower.pin = power.pin / 1000000;
        TRACE_INFO(GetLogger(), "Power of {:s} is : {:d}W",
                   "pin", asicpower.pin);
        asicpower.pout1 = power.pout1 / 1000000;
        TRACE_INFO(GetLogger(), "Power of {:s} is : {:d}W",
                   "pout1", asicpower.pout1);
        asicpower.pout2 = asicpower.pout2 / 1000000;
        TRACE_INFO(GetLogger(), "Power of {:s} is : {:d}W",
                   "pout2", asicpower.pout2);

        //Publish Delphi object
        delphi::objects::AsicPowerMetrics::Publish(key, &asicpower);
    } else {
        TRACE_ERR(GetLogger(), "Reading power failed");
    }
    return;
}

MONFUNC(checkpower);
