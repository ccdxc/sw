//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//

#include <system.h>
#include "node.h"
#include "pdsa_hdlr.h"

uword
flow_monitor_process (vlib_main_t * vm,
                      vlib_node_runtime_t * rt,
                      vlib_frame_t * f)
{
    pds_flow_main_t *fm = &pds_flow_main;

    while(1) {
        vlib_process_wait_for_event_or_clock(vm,
                                             PDS_FLOW_STATS_PUBLISH_INTERVAL);

        pdsa_flow_stats_publish(fm->flow_metrics_hdl,
                                (uint64_t *)fm->stats.counter);
    }
    return 0;
}


/* *INDENT-OFF* */
VLIB_REGISTER_NODE (flow_monitor_node, static) =
{
    .function = flow_monitor_process,
    .type = VLIB_NODE_TYPE_PROCESS,
    .name = "flow-monitor-process",
};
/* *INDENT-ON* */
