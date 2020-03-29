//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//

#include <include/sdk/base.hpp>
#include <lib/pal/pal.hpp>
#include <lib/p4/p4_utils.hpp>
#include <lib/p4/p4_api.hpp>
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

