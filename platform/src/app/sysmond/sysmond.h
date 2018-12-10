/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#ifndef __SYSMOND_H__
#define __SYSMOND_H__

#include <stdio.h>
#include <stdlib.h>
#include "logger.h"
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
