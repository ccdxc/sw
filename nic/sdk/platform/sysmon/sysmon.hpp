/*
 * Copyright (c) 2019, Pensando Systems Inc.
 */

#ifndef __SYSMON_H__
#define __SYSMON_H__

#include "lib/catalog/catalog.hpp"
#include "platform/sensor/sensor.hpp"
#include "platform/pal/include/pal_types.h"

typedef enum {
   CRITICAL_EVENT = 0,
   NON_CRITICAL_EVENT = 1,
   PROCESS_CRASHED_EVENT = 2,
   EVERYTHING_WORKING = 3,
   UKNOWN_STATE = 4
} sysmond_led_event_t;

typedef enum {
   SYSMOND_HBM_TEMP_NONE = 0,
   SYSMOND_HBM_TEMP_ABOVE_THRESHOLD = 1,
   SYSMOND_HBM_TEMP_BELOW_THRESHOLD = 2
} sysmond_hbm_threshold_event_t;

typedef struct systemled_s {
    sysmond_led_event_t event;
    pal_led_color_t color;
} systemled_t;

// callbacks
typedef void (*frequency_change_event_cb_t)(uint32_t frequency);
typedef void (*cattrip_event_cb_t)(void);
typedef void (*power_event_cb_t)(sdk::platform::sensor::system_power_t *power);
typedef void (*temp_event_cb_t)(
        sdk::platform::sensor::system_temperature_t *temperature,
        sysmond_hbm_threshold_event_t hbm_event);
typedef void (*memory_event_cb_t)(
        uint64_t total_mem, uint64_t available_mem, uint64_t free_mem);
typedef void (*panic_event_cb_t)(void);
typedef void (*postdiag_event_cb_t)(void);

typedef struct sysmon_cfg_s {
    frequency_change_event_cb_t frequency_change_event_cb;
    cattrip_event_cb_t          cattrip_event_cb;
    power_event_cb_t            power_event_cb;
    temp_event_cb_t             temp_event_cb;
    memory_event_cb_t           memory_event_cb;
    panic_event_cb_t            panic_event_cb;
    postdiag_event_cb_t         postdiag_event_cb;
    sdk::lib::catalog           *catalog;
} sysmon_cfg_t;

int sysmon_init(sysmon_cfg_t *sysmon_cfg);
void sysmgrsystemled(systemled_t led);
int sysmon_monitor(void);

#endif    // __SYSMON_H__
