//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------
#include "nic/sdk/include/sdk/eth.hpp"
#include "nic/sdk/include/sdk/if.hpp"
#include "nic/apollo/api/include/athena/pds_epoch.h"
#include "nic/apollo/test/athena/api/epoch/utils.hpp"

void
fill_key (pds_epoch_key_t *key, uint32_t index)
{
     key->epoch_id = index;
     return;
}

void
fill_data (pds_epoch_data_t *data, uint32_t index)
{
    data->epoch = index % 65536;
    return;
}

void
update_data (pds_epoch_data_t *data, uint32_t index)
{
    data->epoch = (index + UPDATE_DELTA) % 65536;
    return;
}
