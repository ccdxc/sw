//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "nic/metaswitch/thread/pdsa_thread.hpp"

namespace pds {
static sdk::lib::thread    *g_pds_threads[PDS_THREAD_ID_MAX];

sdk::lib::thread *
pdsa_thread_create (const char *name, uint32_t thread_id,
                    sdk::lib::thread_role_t thread_role,
                    uint64_t cores_mask,
                    sdk::lib::thread_entry_func_t entry_func,
                    uint32_t thread_prio, int sched_policy, void *data)
{
    g_pds_threads[thread_id] =
        sdk::lib::thread::factory(name, thread_id, thread_role, cores_mask,
                                  entry_func, thread_prio, sched_policy,
                                  (thread_role == sdk::lib::THREAD_ROLE_DATA) ?
                                       false : true);
    if (g_pds_threads[thread_id]) {
        g_pds_threads[thread_id]->set_data(data);
    }

    return g_pds_threads[thread_id];
}

}  // namespace pds
