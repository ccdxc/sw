/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#ifndef __OSAL_ASSERT_H__
#define __OSAL_ASSERT_H__

#ifndef __KERNEL__
#include <assert.h>
#define PNSO_ASSERT(x) assert(x)
#else
#define PNSO_ASSERT(x)
#endif

#endif	/* __OSAL_ASSERT_H__ */
