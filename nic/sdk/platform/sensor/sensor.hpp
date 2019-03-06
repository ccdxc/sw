/*
 * Copyright (c) 2017-2018, Pensando Systems Inc.
 */

#ifndef __SENSOR_HPP__
#define __SENSOR_HPP__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include "include/sdk/base.hpp"
#include "third-party/asic/capri/verif/apis/cap_nwl_sbus_api.h"
#include "third-party/asic/capri/verif/apis/cap_sbus_api.h"

namespace sdk {
namespace platform {
namespace sensor {

#define PIN_INPUT_FILE "/sys/class/hwmon/hwmon1/power1_input"
#define POUT1_INPUT_FILE "/sys/class/hwmon/hwmon1/power2_input"
#define POUT2_INPUT_FILE "/sys/class/hwmon/hwmon1/power3_input"
#define LOCAL_TEMP_FILE "/sys/class/hwmon/hwmon0/temp1_input"

typedef struct system_temperature {
    int dietemp;
    int localtemp;
    int hbmtemp;
} system_temperature_t;

typedef struct system_power {
    int pin;
    int pout1;
    int pout2;
} system_power_t;

//Read temperature functions will fill the value in millidegrees
int read_local_temperature(int *localtemp);
int read_temperatures(system_temperature_t *temperature);

//Read power functions will fill the value in microwatts
int read_powers(system_power_t *power);


} // namespace sensor
} // namespace platform
} // namespace sdk

#endif /* __SENSOR_HPP__ */
