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
#include "include/sdk/types.hpp"
#include <boost/property_tree/json_parser.hpp>
#include "sysmon.hpp"

typedef struct monfunc_s {
    void (*func)(void);
} monfunc_t;

typedef enum sysmond_event_e {
    SYSMOND_FREQUENCY_CHANGE,
} sysmond_event_t;

typedef struct sysmond_db_s {
    uint32_t frequency;
} sysmond_db_t;

// event identifiers
typedef enum event_id_e {
    EVENT_ID_NONE               = 0,
    EVENT_ID_HAL_UP             = 1,
    EVENT_ID_PORT_STATUS        = 2,
    EVENT_ID_XCVR_STATUS        = 3,
    EVENT_ID_UPLINK_STATUS      = 4,
    EVENT_ID_HOST_LIF_CREATE    = 5,
    EVENT_ID_LIF_STATUS         = 6,
    EVENT_ID_UPG                = 7,
    EVENT_ID_UPG_STAGE_STATUS   = 8,
    EVENT_ID_MICRO_SEG          = 9,
    EVENT_ID_NCSI               = 10,
} event_id_t;

#define UPTIME_FILENAME "/obfl/uptime.log"
#define MONFUNC(fnc) \
static monfunc_t monfunc_t_##fnc \
    __attribute__((used, section("monfunclist"), aligned(4))) = \
{ \
    .func = fnc, \
}

int configurefrequency(void);
void checkpanicdump(void);
void sysmgrsystemled(systemled_t led);
void checkcattrip(void);
void checkfrequency(void);
void checkruntime(void);
void checkliveness(void);
void checktemperature(void);
void checkdisk(void);
void checkmemory(void);
void checkpower(void);
void checkpostdiag(void);

extern sysmon_cfg_t g_sysmon_cfg;

#endif /*__SYSMOND_H__*/
