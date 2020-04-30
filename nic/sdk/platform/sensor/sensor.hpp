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
#include "platform/drivers/xcvr_qsfp.hpp"
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

#define MAX_PORTS 2
#define DIE_TEMP_STANDARD_DEVIATION 22500

typedef enum asic_temp_metrics_type_e {
    ASIC_TEMP_METRICS_TYPE_LOCAL,
    ASIC_TEMP_METRICS_TYPE_DIE,
    ASIC_TEMP_METRICS_TYPE_HBM,
    ASIC_TEMP_METRICS_TYPE_MAX,
} asic_temp_metrics_type_t;

typedef enum port_temp_metrics_type_e {
    PORT_TEMP_METRICS_TYPE_PORT,
    PORT_TEMP_METRICS_TYPE_WARN,
    PORT_TEMP_METRICS_TYPE_ALARM,
    PORT_TEMP_METRICS_TYPE_MAX,
} port_temp_metrics_type_t;

typedef struct system_temperature {
    int localtemp;
    int dietemp;
    int hbmtemp;
    int hbmwarningtemp;
    int hbmcriticaltemp;
    sdk::platform::qsfp_temperature_t xcvrtemp[MAX_PORTS];
} system_temperature_t;

typedef struct system_power {
    int pin;
    int pout1;
    int pout2;
} __attribute__((packed)) system_power_t;

typedef struct system_voltage {
    int vin;
    int vout1;
    int vout2;
} system_voltage_t;

//Read temperature functions will fill the value in millidegrees
int read_temperatures(system_temperature_t *temperature);
int read_local_temperature(int *localtemp);
int read_voltages(system_voltage_t *voltage);

//Read power functions will fill the value in microwatts
int read_powers(system_power_t *power);

} // namespace sensor
} // namespace platform
} // namespace sdk

using sdk::platform::sensor::asic_temp_metrics_type_t;
using sdk::platform::sensor::port_temp_metrics_type_t;
using sdk::platform::sensor::system_temperature_t;
using sdk::platform::sensor::system_power_t;

#endif /* __SENSOR_HPP__ */
