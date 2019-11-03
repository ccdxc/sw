// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// Purpose: Helper APIs for metaswitch HALS stub programming 

#include "pdsa_mgmt_utils.hpp"
#include "hals_mib.h"


// Fill halsEntTable: AMB_STUBS_HALS_ENT
NBB_VOID
pdsa_fill_amb_hals_ent (AMB_GEN_IPS          *mib_msg,
                        AMB_STUBS_HALS_ENT   *v_amb_hals_ent,
                        NBB_ULONG            entity_index,
                        NBB_LONG             row_status, 
                        NBB_ULONG            stateful)
{
    NBB_TRC_ENTRY ("pdsa_fill_amb_hals_ent");

    // Set all fields absent
    AMB_SET_ALL_FIELDS_NOT_PRESENT (mib_msg);

    // Set all incoming fields
    v_amb_hals_ent->entity_index = entity_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_HALS_ENT_ENTITY_INDEX);

    v_amb_hals_ent->row_status = row_status;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_HALS_ENT_ROW_STATUS);

    if (row_status != AMB_ROW_DESTROY)
    {
        NBB_TRC_FLOW((NBB_FORMAT "Not destroying HALS Ent: fill in fields"));
        v_amb_hals_ent->stateful = stateful;
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_HALS_ENT_STATEFUL);
    }

    NBB_TRC_EXIT ();
    return;
}
