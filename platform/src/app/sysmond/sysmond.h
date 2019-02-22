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

typedef struct monfunc_s {
    void (*func)(void);
} monfunc_t;

#define MONFUNC(fnc) \
static monfunc_t monfunc_t_##fnc \
    __attribute__((used, section("monfunclist"), aligned(4))) = \
{ \
    .func = fnc, \
}
#endif /*__SYSMOND_H__*/
