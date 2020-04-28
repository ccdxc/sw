//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//

#include <system.h>
#include "node.h"
#include "pdsa_hdlr.h"
#include "pdsa_uds_hdlr.h"

void
pds_flow_cfg_set (uint8_t con_track_en,
                  uint32_t tcp_syn_timeout,
                  uint32_t tcp_half_close_timeout,
                  uint32_t tcp_close_timeout,
                  const uint32_t *flow_idle_timeout,
                  const uint32_t *flow_drop_timeout)
{
    pds_flow_main_t *fm = &pds_flow_main;
    int i;

    fm->con_track_en = con_track_en;
    fm->tcp_con_setup_timeout = PDS_FLOW_SEC_TO_TIMER_TICK(tcp_syn_timeout);
    fm->tcp_half_close_timeout =
            PDS_FLOW_SEC_TO_TIMER_TICK(tcp_half_close_timeout);
    fm->tcp_close_timeout =
            PDS_FLOW_SEC_TO_TIMER_TICK(tcp_close_timeout);
    fm->tcp_keep_alive_timeout = PDS_FLOW_SEC_TO_TIMER_TICK(TCP_KEEP_ALIVE_TIMEOUT);

    for (i = PDS_FLOW_PROTO_START; i < PDS_FLOW_PROTO_END; i++) {
        vec_elt(fm->idle_timeout, i) =
                PDS_FLOW_SEC_TO_TIMER_TICK(flow_idle_timeout[i]);
        vec_elt(fm->idle_timeout_ticks, i) =
                pds_system_get_ticks(flow_idle_timeout[i]);
        vec_elt(fm->drop_timeout, i) =
                PDS_FLOW_SEC_TO_TIMER_TICK(flow_drop_timeout[i]);
        vec_elt(fm->drop_timeout_ticks, i) =
                        pds_system_get_ticks(flow_drop_timeout[i]);
    }
}
