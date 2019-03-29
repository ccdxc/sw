/*
 * Copyright (c) 2019, Pensando Systems Inc.
 */

#ifndef __SYSMOND_H__
#define __SYSMOND_H__

#include <stdio.h>
#include <stdlib.h>
#include <memory>
#include "logger.h"
#include "lib/thread/thread.hpp"
#include "nic/sdk/lib/pal/pal.hpp"
#include "nic/sdk/platform/pal/include/pal.h"
#include "gen/proto/sysmond.delphi.hpp"
#include "nic/sdk/include/sdk/base.hpp"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include "third-party/asic/capri/verif/apis/cap_freq_api.h"

typedef struct monfunc_s {
    void (*func)(void);
} monfunc_t;

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

#define MONFUNC(fnc) \
static monfunc_t monfunc_t_##fnc \
    __attribute__((used, section("monfunclist"), aligned(4))) = \
{ \
    .func = fnc, \
}

void sysmgrsystemled (systemled_t led);
#endif /*__SYSMOND_H__*/
