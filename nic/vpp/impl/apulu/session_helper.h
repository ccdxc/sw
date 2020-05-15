//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//

#ifndef __VPP_IMPL_APULU_SESSION_HELPER_H__
#define __VPP_IMPL_APULU_SESSION_HELPER_H__

#include <nic/vpp/flow/node.h>
#include <nic/apollo/p4/include/apulu_defines.h>

always_inline bool
pds_flow_from_host (u32 ses_id, u8 flow_role)
{
    pds_flow_hw_ctx_t *session = pds_flow_get_hw_ctx(ses_id);
    if (pds_flow_packet_l2l(session->packet_type)) {
        //both flows are from host
        return true;
    }

    // If iflow_rx is true, then iflow is from uplink and rflow is from host
    // and vice versa
    if (flow_role == TCP_FLOW_INITIATOR) {
        return !session->iflow_rx;
    }
    return session->iflow_rx;
}

#endif    // __VPP_IMPL_APULU_SESSION_HELPER_H__
