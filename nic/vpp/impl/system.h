//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
// Capri system related info interfaces

#ifndef __VPP_IMPL_SYSTEM_H__
#define __VPP_IMPL_SYSTEM_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

uint64_t pds_system_get_ticks(uint64_t sec);

double pds_system_get_secs(uint64_t tick);

uint64_t pds_system_get_current_tick(void);

#ifdef __cplusplus
}
#endif

#endif // __VPP_IMPL_SYSTEM_H__
