//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//

#include "impl_db.h"
#include "pdsa_impl_db_hdlr.h"

bool
pds_session_active_on_vnic_get (uint16_t vnic_id, uint32_t *sess_count)
{
    pds_impl_db_vnic_entry_t *vnic_info = NULL;

    vnic_info = pds_impl_db_vnic_get(vnic_id);
    if (vnic_info == NULL) {
        return false;
    }

    *sess_count = vnic_info->active_ses_count;

    return true;
}



