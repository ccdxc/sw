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
#define VIN_INPUT_FILE "/sys/class/hwmon/hwmon1/in1_input"
#define VOUT1_INPUT_FILE "/sys/class/hwmon/hwmon1/in2_input"
#define VOUT2_INPUT_FILE "/sys/class/hwmon/hwmon1/in3_input"
#define LOCAL_TEMP_FILE "/sys/class/hwmon/hwmon0/temp1_input"
#define DIE_TEMP_FILE "/sys/class/hwmon/hwmon0/temp2_input"

#define DIE_TEMP_STANDARD_DEVIATION 22500

typedef struct system_temperature {
    int dietemp;
    int localtemp;
    int hbmtemp;
    int qsfp1temp;
    int qsfp2temp;
    int qsfp1warningtemp;
    int qsfp2warningtemp;
    int qsfp1alarmtemp;
    int qsfp2alarmtemp;
} system_temperature_t;

typedef struct system_power {
    int pin;
    int pout1;
    int pout2;
} system_power_t;

typedef struct system_voltage {
    int vin;
    int vout1;
    int vout2;
} system_voltage_t;

//Read temperature functions will fill the value in millidegrees
int read_local_temperature(int *localtemp);
int read_temperatures(system_temperature_t *temperature);
int read_voltages(system_voltage_t *voltage);

//Read power functions will fill the value in microwatts
int read_powers(system_power_t *power);

} // namespace sensor
} // namespace platform
} // namespace sdk

#endif /* __SENSOR_HPP__ */
