#ifndef _SYSMOND_EVENTRECORDER_CB_H_
#define _SYSMOND_EVENTRECORDER_CB_H_

#include "platform/sysmon/sysmon.hpp"
#include "nic/sdk/platform/sensor/sensor.hpp"

void eventrecorder_cattrip_event_cb(void);
void eventrecorder_temp_event_cb(
                sdk::platform::sensor::system_temperature_t *temperature,
                sysmond_hbm_threshold_event_t hbm_event);
void eventrecorder_fatal_interrupt_event_cb(const char *desc);

#endif    // _SYSMOND_EVENTRECORDER_CB_H_
