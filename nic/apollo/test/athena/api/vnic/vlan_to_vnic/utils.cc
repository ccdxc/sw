//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------
#include "nic/sdk/include/sdk/eth.hpp"
#include "nic/sdk/include/sdk/if.hpp"
#include "nic/apollo/api/include/athena/pds_vnic.h"
#include "nic/apollo/test/athena/api/vnic/vlan_to_vnic/utils.hpp"

void
fill_vlan_to_vnic_key (pds_vlan_to_vnic_map_key_t *key, uint32_t index)
{
     key->vlan_id = index;
     return;
}

void
fill_vlan_to_vnic_data (pds_vlan_to_vnic_map_data_t *data, uint32_t index)
{
    data->vnic_id = index % PDS_VNIC_ID_MAX;
    data->vnic_type = VNIC_TYPE_L3;
    return;
}

void
update_vlan_to_vnic_data (pds_vlan_to_vnic_map_data_t *data, uint32_t index)
{
    data->vnic_id = (index + UPDATE_DELTA) % PDS_VNIC_ID_MAX;
    data->vnic_type = VNIC_TYPE_L2;
    return;
}
