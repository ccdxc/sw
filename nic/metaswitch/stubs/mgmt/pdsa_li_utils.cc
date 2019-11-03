// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// Purpose: Helper APIs for metaswitch LI stub programming 

#include "pdsa_mgmt_utils.hpp"
#include "li_mgmt_if.h"



// Fill liEntTable: AMB_STUBS_LI_ENT
NBB_VOID
pdsa_fill_amb_li_ent (AMB_GEN_IPS      *mib_msg,
                      AMB_STUBS_LI_ENT *v_amb_li_ent,
                      NBB_ULONG         entity_index,
                      NBB_LONG          row_status,
                      NBB_LONG          stateful)
{ 
    NBB_TRC_ENTRY ("pdsa_fill_amb_li_ent");

    // Set all fields absent
    AMB_SET_ALL_FIELDS_NOT_PRESENT (mib_msg);

    // Set all incoming fields
    v_amb_li_ent->entity_index = entity_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_LI_ENT_ENTITY_INDEX);

    v_amb_li_ent->row_status = row_status;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_LI_ENT_ROW_STATUS);

    if (row_status != AMB_ROW_DESTROY)
    {
        NBB_TRC_FLOW((NBB_FORMAT "Not destroying LI Ent: fill in fields"));
        v_amb_li_ent->stateful = stateful;
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_LI_ENT_STATEFUL);
    }
    NBB_TRC_EXIT ();
    return;
}

// Fill liMjTable: AMB_STUBS_LI_MJ
NBB_VOID
pdsa_fill_amb_li_mj (AMB_GEN_IPS        *mib_msg,
                     AMB_STUBS_LI_MJ    *v_amb_li_mj,
                     NBB_ULONG          li_entity_index,
                     NBB_LONG           interface_id,
                     NBB_ULONG          partner_type,
                     NBB_ULONG          partner_index,
                     NBB_ULONG          sub_index,
                     NBB_LONG           row_status)
{ 
    NBB_TRC_ENTRY ("pdsa_fill_amb_li_mj");

    // Set all fields absent
    AMB_SET_ALL_FIELDS_NOT_PRESENT (mib_msg);

    // Set all incoming fields
    v_amb_li_mj->li_entity_index = li_entity_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_LI_MJ_ENTITY_INDEX);

    v_amb_li_mj->interface_id = interface_id;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_LI_MJ_INTERFACE);

    v_amb_li_mj->partner_type = partner_type;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_LI_MJ_PARTNER_TYPE);

    v_amb_li_mj->partner_index = partner_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_LI_MJ_PARTNER_INDEX);

    v_amb_li_mj->sub_index = sub_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_LI_MJ_SUB_INDEX);

    v_amb_li_mj->row_status = row_status;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_LI_MJ_ROW_STATUS);

    NBB_TRC_EXIT ();
    return;
}
