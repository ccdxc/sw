//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//

#include <stdint.h>
#include <nic/vpp/impl/session.h>

uint64_t
pds_session_get_timestamp (uint32_t session_id)
{
    return ~0;
}

void
pds_session_get_session_state (uint32_t session_id, uint8_t *iflow_state, 
                               uint8_t *rflow_state)
{
    return;
}

bool
pds_session_state_established (uint8_t state)
{
    return false;
}

bool
pds_session_active_on_vnic_get (uint16_t vnic_id, uint32_t *sess_count)
{
    return false;
}
