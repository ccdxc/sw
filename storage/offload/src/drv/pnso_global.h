/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#ifndef __PNSO_GLOBAL_H__
#define __PNSO_GLOBAL_H__

#include <assert.h>
#include <errno.h>

#include <stdio.h>
#include <stddef.h>
#include <string.h>

#include <unistd.h>

#include "pnso_types.h"
#include "pnso_api.h"
#include "pnso_osal.h"

/* TODO: remove these from here! */
#define PNSO_UT_MAX_DBUFS	16
#define PNSO_UT_ALIGN_DBUFS	4096

#define PNSO_UT_NUM_POLL	(5)
#define PNSO_UT_POLL_DELAY	(20 * 1000000)

#define MAX_CPDC_BUF_LEN	(1 << 15)
// #define MAX_CPDC_BUF_LEN	32

#define PNSO_MEM_ALIGN_SPEC	64
#define PNSO_MEM_ALIGN_BUF	256
#define PNSO_MEM_ALIGN_PAGE	4096

/* TODO: do away with hacks ... */
#define USE_DPOOL	1
#define CHAIN_BY_NAME	1

#endif	/* __PNSO_GLOBAL_H__ */
