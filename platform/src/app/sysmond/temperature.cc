/*
 * Copyright (c) 2019, Pensando Systems Inc.
 */

#include "sysmond.h"
#include "nic/sdk/platform/sensor/sensor.hpp"

static delphi::objects::asictemperaturemetrics_t    asictemp;

static void
checktemperature(void)
{
    uint8_t key = 0;
    uint32_t ret;
    sdk::platform::sensor::system_temperature_t temperature;

    ret = sdk::platform::sensor::read_temperatures(&temperature);
    if (!ret) {
        asictemp.die_temperature = temperature.dietemp / 1000;
        TRACE_INFO(GetLogger(), "{:s} is : {:d}C",
                   "Die temperature", asictemp.die_temperature);
        asictemp.local_temperature = temperature.localtemp / 1000;
        TRACE_INFO(GetLogger(), "{:s} is : {:d}C",
                   "Local temperature", asictemp.local_temperature);
        asictemp.hbm_temperature = temperature.hbmtemp;
        TRACE_INFO(GetLogger(), "HBM temperature is : {:d}C", asictemp.hbm_temperature);

        //Publish Delphi object
        delphi::objects::AsicTemperatureMetrics::Publish(key, &asictemp);
    } else {
        TRACE_ERR(GetLogger(), "Reading temperature failed");
    }

    return;
}

MONFUNC(checktemperature);
