#ifndef _SYSMOND_DELPHI_CB_H_
#define _SYSMOND_DELPHI_CB_H_

#include "nic/sdk/platform/sensor/sensor.hpp"

void delphi_event_cb_init(void);
void delphi_frequency_change_event_cb(uint32_t frequency);
void delphi_cattrip_event_cb(void);
void delphi_power_event_cb(sdk::platform::sensor::system_power_t *power);
void delphi_temp_event_cb(sdk::platform::sensor::system_temperature_t *temperature);
void delphi_memory_event_cb(uint64_t total_mem, uint64_t available_mem,
                     uint64_t free_mem);

#endif    // _SYSMOND_DELPHI_CB_H_
