/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */

#ifndef __PNSO_TEST_DEV_H__
#define __PNSO_TEST_DEV_H__

#include "pnso_api.h"
#include "osal_logger.h"

#ifdef __cplusplus
extern "C"
{
#endif

int pnso_test_cdev_init(void);
void pnso_test_cdev_deinit(void);

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* __PNSO_TEST_DEV_H__ */
