#ifndef _SYSMOND_CB_H_
#define _SYSMOND_CB_H_

#include "nic/sdk/platform/sensor/sensor.hpp"

void event_cb_init(void);
void frequency_change_event_cb(uint32_t frequency);
void cattrip_event_cb(void);
void power_event_cb(sdk::platform::sensor::system_power_t *power);
void temp_event_cb(sdk::platform::sensor::system_temperature_t *temperature);
void memory_event_cb(uint64_t total_mem, uint64_t available_mem,
                     uint64_t free_mem);

#endif    // _SYSMOND_CB_H_
