//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//

#include "node.h"

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
