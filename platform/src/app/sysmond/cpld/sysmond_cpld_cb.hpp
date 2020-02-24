#ifndef _SYSMOND_CPLD_CB_H_
#define _SYSMOND_CPLD_CB_H_

#include "nic/sdk/platform/sensor/sensor.hpp"

void cpld_temp_event_cb(
                sdk::platform::sensor::system_temperature_t *temperature);
void cpld_liveness_event_cb(void);

#endif    // _SYSMOND_CPLD_CB_H_
