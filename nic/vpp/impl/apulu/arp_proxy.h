//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//

#ifndef __VPP_IMPL_APULU_ARP_PROXY_H__
#define __VPP_IMPL_APULU_ARP_PROXY_H__

#include <nic/apollo/api/impl/apulu/nacl_data.h>
#include <impl_db.h>
#include <api.h>

always_inline void
pds_arp_proxy_pipeline_init (void)
{
    pds_infra_api_reg_t params = {0};

    params.nacl_id = NACL_DATA_ID_FLOW_MISS_ARP;
    params.node = format(0, "pds-arp-proxy");
    params.frame_queue_index = ~0;
    params.handoff_thread = ~0;
    params.offset = 0;
    params.unreg = 0;

    if (0 != pds_register_nacl_id_to_node(&params)) {
        ASSERT(0);
    }
    return;
}

#endif      //__VPP_IMPL_APULU_ARP_PROXY_H__
