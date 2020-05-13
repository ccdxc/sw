//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This module defines nicmgr shared memory API
///
//----------------------------------------------------------------------------

#ifndef __NICMGR_SHM_HPP__
#define __NICMGR_SHM_HPP__

#include <sys/types.h>
#include <unistd.h>
#include "nic/sdk/include/sdk/base.hpp"
#include "nic/apollo/api/include/pds_init.hpp"
#include "nic/sdk/lib/shmmgr/shmmgr.hpp"
#include "platform/src/lib/nicmgr/include/device.hpp"

sdk_ret_t nicmgr_shm_init(pds_init_params_t *params);
sdk_ret_t nicmgr_shm_cpp_pid_set(enum DeviceType type,
                                 pid_t pid);
void nicmgr_shm_cpp_pid_clr(enum DeviceType type);
bool nicmgr_shm_is_cpp_pid(enum DeviceType type);
bool nicmgr_shm_is_cpp_pid_any_dev(void);
sdk_ret_t nicmgr_shm_base_lif_id_set(enum DeviceType type,
                                     uint32_t base_lif_id);
void nicmgr_shm_base_lif_id_clr(enum DeviceType type);
uint32_t nicmgr_shm_base_lif_id(enum DeviceType type);
sdk_ret_t nicmgr_shm_lif_fully_created_set(enum DeviceType type);
void nicmgr_shm_lif_fully_created_clr(enum DeviceType type);
bool nicmgr_shm_lif_fully_created(enum DeviceType type);

#endif // __NICMGR_SHM_HPP__
