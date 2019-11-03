// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// Purpose: Helper APIs for metaswitch Sock stub programming 

#include "pdsa_mgmt_utils.hpp"
#include "sck0mib.h"

// Fill sckTable: AMB_STUBS_SCK
NBB_VOID
pdsa_fill_amb_sck (AMB_GEN_IPS      *mib_msg,
                   AMB_STUBS_SCK    *v_amb_sck,
                   NBB_ULONG        index,
                   NBB_LONG         row_status)
{
    NBB_TRC_ENTRY ("pdsa_fill_amb_sck");

    // Set all fields absent
    AMB_SET_ALL_FIELDS_NOT_PRESENT (mib_msg);
    
    // Set all incoming fields
    v_amb_sck->index = index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_STUBS_SCK_INDEX);

    v_amb_sck->row_status = row_status;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_STUBS_SCK_ROW_STATUS);

    NBB_TRC_EXIT ();
    return;
}
