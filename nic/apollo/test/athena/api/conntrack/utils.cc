//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------
#include "nic/sdk/include/sdk/eth.hpp"
#include "nic/sdk/include/sdk/if.hpp"
#include "nic/apollo/api/include/athena/pds_conntrack.h"
#include "nic/apollo/test/athena/api/conntrack/utils.hpp"

void
fill_key (pds_conntrack_key_t *key, uint32_t index)
{
     key->conntrack_id = index;
     return;
}

void
fill_data (pds_conntrack_data_t *data, uint32_t index)
{
    data->flow_type = PDS_FLOW_TYPE_TCP;
    data->flow_state = (pds_flow_state_t)((index % 11) + 1);
    return;
}

void
update_data (pds_conntrack_data_t *data, uint32_t index)
{
    data->flow_type = PDS_FLOW_TYPE_OTHERS;
    data->flow_state = UNESTABLISHED;
    return;
}
