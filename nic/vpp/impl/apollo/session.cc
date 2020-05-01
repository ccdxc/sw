//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//

#include <stdint.h>
#include <nic/vpp/impl/session.h>
#include <pds_table.h>

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

int
session_track_program(uint32_t ses_id, void *action)
{
    return 0;
}

void
pds_session_get_info (uint32_t session_id, session_info_t *info)
{
    return;
}

void
pds_session_track_get_info (uint32_t session_id, session_track_info_t *info)
{
    return;
}

bool
pds_session_get_xlate_ids (uint32_t ses, uint16_t *rx_xlate_id,
                           uint16_t *tx_xlate_id,
                           uint16_t *rx_xlate_id2,
                           uint16_t *tx_xlate_id2)
{
    return false;
}

int
pds_session_stats_read(uint32_t session_id, pds_session_stats_t *stats)
{
    return -1;
}

int
pds_session_stats_clear (uint32_t session_id)
{
    return -1;
}
