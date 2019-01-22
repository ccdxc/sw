/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#include "sysmond.h"
#include "nic/asic/capri/verif/apis/cap_nwl_sbus_api.h"
#include "nic/sdk/platform/sensor/sensor.hpp"

static delphi::objects::asictemperaturemetrics_t    asictemp;

static void
checktemperature(void)
{
    uint8_t counter = 0;
    uint8_t key = 0;
    uint32_t ret;

    ret = sdk::platform::sensor::read_die_temperature(&asictemp.die_temperature);
    if (!ret) {
        asictemp.die_temperature = asictemp.die_temperature / 1000;
        TRACE_INFO(GetLogger(), "{:s} is : {:d}W",
                   "Die temperature", asictemp.die_temperature);
    } else if (ret != ENOENT){
        TRACE_ERR(GetLogger(), "Reading die temperature failed");
    }

    ret = sdk::platform::sensor::read_local_temperature(&asictemp.local_temperature);
    if (!ret) {
        asictemp.local_temperature = asictemp.local_temperature / 1000;
        TRACE_INFO(GetLogger(), "{:s} is : {:d}W",
                   "Local temperature", asictemp.local_temperature);
    } else if (ret != ENOENT){
        TRACE_ERR(GetLogger(), "Reading local temperature failed");
    }

    //check HBM temperature
    asictemp.hbm_temperature = cap_nwl_sbus_get_1500_temperature();
    TRACE_INFO(GetLogger(), "HBM temperature is : {:d}C", asictemp.hbm_temperature);

    delphi::objects::AsicTemperatureMetrics::Publish(key, &asictemp);
    return;
}

MONFUNC(checktemperature);
