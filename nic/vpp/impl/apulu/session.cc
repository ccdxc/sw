//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//

#include <include/sdk/base.hpp>
#include <lib/pal/pal.hpp>
#include <lib/p4/p4_utils.hpp>
#include <lib/p4/p4_api.hpp>
#include <nic/apollo/p4/include/apulu_defines.h>
#include <pd_utils.h>
#include <nic/vpp/impl/session.h>

uint64_t
pds_session_get_timestamp (uint32_t session_id)
{
    p4pd_error_t p4pd_ret;
    session_swkey_t key;
    session_actiondata_t ses_data;
    uint64_t *timestamp;
    volatile uint64_t ret;

    key.p4e_i2e_session_id = session_id;
    p4pd_ret = p4pd_global_entry_read(P4TBL_ID_SESSION, key.p4e_i2e_session_id,
                                      NULL, NULL, &ses_data);
    assert(p4pd_ret == P4PD_SUCCESS);

    timestamp = (uint64_t *) ses_data.action_u.session_session_info.timestamp;
    ret = ((*timestamp) << 16) >> 16;
    return ret;
}

void
pds_session_get_session_state (uint32_t session_id, uint8_t *iflow_state, 
                               uint8_t *rflow_state)
{
    p4pd_error_t p4pd_ret;
    session_track_swkey_t key;
    session_track_actiondata_t ses_data;

    key.p4e_i2e_session_id = session_id;
    p4pd_ret = p4pd_global_entry_read(P4TBL_ID_SESSION_TRACK, key.p4e_i2e_session_id,
                                      NULL, NULL, &ses_data);
    assert(p4pd_ret == P4PD_SUCCESS);

    *iflow_state = ses_data.action_u.session_track_session_track_info.iflow_tcp_state;
    *rflow_state = ses_data.action_u.session_track_session_track_info.rflow_tcp_state;
}

bool
pds_session_state_established (uint8_t state)
{
    return (state == FLOW_STATE_ESTABLISHED);
}

int
session_track_program(uint32_t ses_id, void *action)
{
    p4pd_error_t p4pd_ret0;
    uint32_t tableid = P4TBL_ID_SESSION_TRACK;

    p4pd_ret0 = p4pd_global_entry_write(tableid, ses_id,
                                        NULL, NULL, action);
    if (p4pd_ret0 != P4PD_SUCCESS) {
        return -1;
    }
    return 0;
}

void
pds_session_get_info (uint32_t session_id, session_info_t *info)
{
    p4pd_error_t p4pd_ret;
    session_track_swkey_t key;
    session_track_actiondata_t ses_data;

    key.p4e_i2e_session_id = session_id;
    p4pd_ret = p4pd_global_entry_read(P4TBL_ID_SESSION_TRACK, key.p4e_i2e_session_id,
                                      NULL, NULL, &ses_data);
    assert(p4pd_ret == P4PD_SUCCESS);

    info->iflow_tcp_seq_num =
        ses_data.action_u.session_track_session_track_info.iflow_tcp_seq_num;
    info->iflow_tcp_ack_num =
        ses_data.action_u.session_track_session_track_info.iflow_tcp_ack_num;
    info->iflow_tcp_win_size =
        ses_data.action_u.session_track_session_track_info.iflow_tcp_win_size;
    info->rflow_tcp_seq_num =
        ses_data.action_u.session_track_session_track_info.rflow_tcp_seq_num;
    info->rflow_tcp_ack_num =
        ses_data.action_u.session_track_session_track_info.rflow_tcp_ack_num;
    info->rflow_tcp_win_size =
        ses_data.action_u.session_track_session_track_info.rflow_tcp_win_size;

    return;
}
