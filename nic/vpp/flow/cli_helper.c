//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//

#include <vlib/vlib.h>
#include "node.h"

int clear_all_flow_entries()
{
    int ret1 = 0, ret2 = 0;
    pds_flow_main_t *fm = &pds_flow_main;
    vlib_main_t *vm = vlib_get_main();

    vlib_worker_thread_barrier_sync(vm);
    ret1 = ftlv4_clear(fm->table4, true, false);
    ret2 = ftlv6_clear(fm->table6, true, false);
    pds_session_id_flush();
    vlib_worker_thread_barrier_release(vm);

    return ret1 && ret2;
}
