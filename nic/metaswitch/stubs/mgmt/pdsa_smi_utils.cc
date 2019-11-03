// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// Purpose: Helper APIs for metaswitch SMI component

#include "pdsa_mgmt_utils.hpp"
#include "smi_mib.h"


// Fill smiTable: AMB_STUBS_SMI_ENT
NBB_VOID
pdsa_fill_amb_smi_ent (AMB_GEN_IPS          *mib_msg,
                       AMB_STUBS_SMI_ENT    *v_amb_smi_ent,
                       NBB_ULONG            entity_index,
                       NBB_LONG             row_status)
{
    NBB_TRC_ENTRY ("pdsa_fill_amb_smi_ent");

    // Set all fields absent
    AMB_SET_ALL_FIELDS_NOT_PRESENT (mib_msg);

    // Set all incoming fields
    v_amb_smi_ent->entity_index = entity_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_SMI_ENT_ENTITY_INDEX);

    v_amb_smi_ent->row_status = row_status;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_SMI_ENT_ROW_STATUS);

    NBB_TRC_EXIT ();
    return;
}
