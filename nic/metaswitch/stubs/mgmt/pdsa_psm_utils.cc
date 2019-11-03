// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// Purpose: Helper APIs for metaswitch PSM stub programming 

#include "pdsa_mgmt_utils.hpp"
#include "psm_mgmt_if.h"


// Fill psmEntTable: AMB_PSM_ENT
NBB_VOID
pdsa_fill_amb_psm_ent (AMB_GEN_IPS          *mib_msg,
                       AMB_PSM_ENT          *v_amb_psm_ent,
                       NBB_LONG             entity_index,
                       NBB_LONG             row_status)
{
    NBB_TRC_ENTRY ("pdsa_fill_amb_psm_ent");

    // Set all fields absent
    AMB_SET_ALL_FIELDS_NOT_PRESENT (mib_msg);

    // Set all incoming fields
    v_amb_psm_ent->entity_index = entity_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_PSM_ENT_ENTITY_INDEX);

    v_amb_psm_ent->row_status = row_status;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_PSM_ENT_ROW_STATUS);

    NBB_TRC_EXIT ();
    return;
}

// Fill psmMjTable: AMB_PSM_MJ
NBB_VOID
pdsa_fill_amb_psm_mj (AMB_GEN_IPS   *mib_msg,
                      AMB_PSM_MJ    *v_amb_psm_mj,
                      NBB_ULONG     psm_entity_index,
                      NBB_LONG      interface_id,
                      NBB_LONG      partner_type,
                      NBB_ULONG     partner_index,
                      NBB_ULONG     sub_index,
                      NBB_LONG      row_status)
{ 
    NBB_TRC_ENTRY ("pdsa_fill_amb_psm_mj");

    // Set all fields absent
    AMB_SET_ALL_FIELDS_NOT_PRESENT (mib_msg);

    // Set all incoming fields
    v_amb_psm_mj->psm_entity_index = psm_entity_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_PSM_MJ_ENTITY_INDEX);

    v_amb_psm_mj->interface_id = interface_id;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_PSM_MJ_INTERFACE);

    v_amb_psm_mj->partner_type = partner_type;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_PSM_MJ_PARTNER_TYPE);

    v_amb_psm_mj->partner_index = partner_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_PSM_MJ_PARTNER_INDEX);

    v_amb_psm_mj->sub_index = sub_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_PSM_MJ_SUB_INDEX);

    v_amb_psm_mj->row_status = row_status;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_PSM_MJ_ROW_STATUS);

    NBB_TRC_EXIT ();
    return;
}
