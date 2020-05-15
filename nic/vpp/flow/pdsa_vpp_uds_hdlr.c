//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//

#include "node.h"
#include "session_helper.h"

// Return pointer to FTL v4 flow table
void * pds_flow_get_table4 (void)
{
    pds_flow_main_t *fm = &pds_flow_main;

    return fm->table4;
}

// Return pointer to FTL v6 flow table
void * pds_flow_get_table6_or_l2 (void)
{
    pds_flow_main_t *fm = &pds_flow_main;

    return fm->table6_or_l2;
}

bool pds_flow_get_session_drop (u32 ses_id)
{
    pds_flow_hw_ctx_t *session = pds_flow_get_hw_ctx(ses_id);
    return session->drop;
}

bool pds_flow_get_flow_from_host (u32 ses_id, u8 flow_role)
{
    return pds_flow_from_host(ses_id, flow_role);
}
