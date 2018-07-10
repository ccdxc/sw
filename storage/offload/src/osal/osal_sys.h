/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */

#ifndef OSAL_SYS_H
#define OSAL_SYS_H

#ifdef __cplusplus
extern "C" {
#endif

int osal_get_coreid(void);
int osal_get_core_count(void);
int osal_yield(void);

#ifdef __cplusplus
}
#endif

#endif
