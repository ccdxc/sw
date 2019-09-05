/*
 * Copyright (c) 2019, Pensando Systems Inc.
 */

#ifndef __SYSMOND_H__
#define __SYSMOND_H__

#include <stdio.h>
#include <stdlib.h>
#include <memory>
#include "lib/thread/thread.hpp"
#include "lib/pal/pal.hpp"
#include "platform/pal/include/pal.h"
#include "include/sdk/base.hpp"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include "lib/catalog/catalog.hpp"
#include "platform/sensor/sensor.hpp"

extern sdk::lib::catalog *catalog;

typedef struct monfunc_s {
    void (*func)(void);
} monfunc_t;

typedef enum sysmond_event_e {
    SYSMOND_FREQUENCY_CHANGE,
} sysmond_event_t;

typedef struct sysmond_db_s {
    uint32_t frequency;
} sysmond_db_t;

typedef enum {
   CRITICAL_EVENT = 0,
   NON_CRITICAL_EVENT = 1,
   PROCESS_CRASHED_EVENT = 2,
   EVERYTHING_WORKING = 3,
   UKNOWN_STATE = 4
} sysmond_led_event_t;

typedef struct systemled_s {
    sysmond_led_event_t event;
    pal_led_color_t color;
} systemled_t;

#define UPTIME_FILENAME "/obfl/uptime.log"
#define MONFUNC(fnc) \
static monfunc_t monfunc_t_##fnc \
    __attribute__((used, section("monfunclist"), aligned(4))) = \
{ \
    .func = fnc, \
}

void checkpanicdump(void);
void sysmgrsystemled(systemled_t led);
int configurefrequency(void);

int sysmond_monitor(void);
int sysmond_init(void);

// callbacks
void event_cb_init(void);
void frequency_change_event_cb(uint32_t frequency);
void cattrip_event_cb(void);
void power_event_cb(sdk::platform::sensor::system_power_t *power);
void temp_event_cb (sdk::platform::sensor::system_temperature_t *temperature);
void memory_event_cb(uint64_t total_mem, uint64_t available_mem,
                     uint64_t free_mem);

void checkcattrip(void);
void checkfrequency(void);
void checkruntime(void);
void checktemperature(void);
void checkdisk(void);
void checkmemory(void);
void checkpower(void);

#endif /*__SYSMOND_H__*/
