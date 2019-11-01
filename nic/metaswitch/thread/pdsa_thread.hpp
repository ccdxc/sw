//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include <iostream>
#include "nic/sdk/lib/utils/utils.hpp"
#include "nic/sdk/lib/logger/logger.hpp"
#include "nic/sdk/lib/device/device.hpp"
#include "nic/metaswitch/include/pds_cfg.hpp"
#include "nic/metaswitch/thread/pds.hpp"

namespace pds {
sdk::lib::thread *
pdsa_thread_create (const char *name, uint32_t thread_id,
                    sdk::lib::thread_role_t thread_role,
                    uint64_t cores_mask,
                    sdk::lib::thread_entry_func_t entry_func,
                    uint32_t thread_prio, int sched_policy, void *data);

}  // namespace pds
