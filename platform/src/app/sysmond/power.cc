/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#include "sysmond.h"
#include "platform/src/lib/sensor/sensor.h"

#define PIN_INPUT_FILE "/sys/class/hwmon/hwmon1/power1_input"
#define POUT1_INPUT_FILE "/sys/class/hwmon/hwmon1/power2_input"
#define POUT2_INPUT_FILE "/sys/class/hwmon/hwmon1/power3_input"

static delphi::objects::asicpowermetrics_t    asicpower;

static void
checkpower(void)
{
    uint8_t counter = 0;
    uint8_t key = 0;
    uint32_t ret;

    ret = read_pin(&asicpower.pin);
    if (!ret) {
        asicpower.pin = asicpower.pin / 1000000;
        TRACE_INFO(GetLogger(), "Power of {:s} is : {:d}W",
                   "pin", asicpower.pin);
    } else if (ret != ENOENT){
        TRACE_ERR(GetLogger(), "Reading PIN failed");
    }

    ret = read_pout1(&asicpower.pout1);
    if (!ret) {
        asicpower.pout1 = asicpower.pout1 / 1000000;
        TRACE_INFO(GetLogger(), "Power of {:s} is : {:d}W",
                   "pout1", asicpower.pout1);
    } else if (ret != ENOENT){
        TRACE_ERR(GetLogger(), "Reading POUT1 failed");
    }

    ret = read_pout2(&asicpower.pout2);
    if (!ret) {
        asicpower.pout2 = asicpower.pout2 / 1000000;
        TRACE_INFO(GetLogger(), "Power of {:s} is : {:d}W",
                   "pout2", asicpower.pout2);
    } else if (ret != ENOENT){
        TRACE_ERR(GetLogger(), "Reading POUT2 failed");
    }

    delphi::objects::AsicPowerMetrics::Publish(key, &asicpower);

    return;
}

MONFUNC(checkpower);
