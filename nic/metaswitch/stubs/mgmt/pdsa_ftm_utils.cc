// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//Purpose: Helper APIs for metaswitch FTM stub programming 

#include "pdsa_mgmt_utils.hpp"
#include "ftm_mgmt_if.h"


// Fill ftmEntTable: AMB_FTM_ENT
NBB_VOID
pdsa_fill_amb_ftm_ent (AMB_GEN_IPS      *mib_msg,
                       AMB_FTM_ENT      *v_amb_ftm_ent,
                       NBB_ULONG        entity_index,
                       NBB_LONG         row_status)
{
    NBB_TRC_ENTRY ("pdsa_fill_amb_ftm_ent");

    // Set all fields absent
    AMB_SET_ALL_FIELDS_NOT_PRESENT (mib_msg);

    // Set all incoming fields
    v_amb_ftm_ent->entity_index = entity_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_FTM_ENT_ENTITY_INDEX);

    v_amb_ftm_ent->row_status = row_status;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_FTM_ENT_ROW_STATUS);

    NBB_TRC_EXIT ();
    return;
}

// Fill ftmMjTable: AMB_FTM_MJ
NBB_VOID
pdsa_fill_amb_ftm_mj (AMB_GEN_IPS   *mib_msg,
                      AMB_FTM_MJ    *v_amb_ftm_mj,
                      NBB_ULONG     ftm_entity_index,
                      NBB_LONG      interface_id,
                      NBB_LONG      partner_type,
                      NBB_ULONG     partner_index,
                      NBB_ULONG     sub_index,
                      NBB_LONG      row_status)
{ 
    NBB_TRC_ENTRY ("pdsa_fill_amb_ftm_mj");

    // Set all fields absent
    AMB_SET_ALL_FIELDS_NOT_PRESENT (mib_msg);

    // Set all incoming fields
    v_amb_ftm_mj->ftm_entity_index = ftm_entity_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_FTM_MJ_ENTITY_INDEX);

    v_amb_ftm_mj->interface_id = interface_id;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_FTM_MJ_INTERFACE);

    v_amb_ftm_mj->partner_type = partner_type;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_FTM_MJ_PARTNER_TYPE);

    v_amb_ftm_mj->partner_index = partner_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_FTM_MJ_PARTNER_INDEX);

    v_amb_ftm_mj->sub_index = sub_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_FTM_MJ_SUB_INDEX);

    v_amb_ftm_mj->row_status = row_status;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_FTM_MJ_ROW_STATUS);

    NBB_TRC_EXIT ();
    return;
}
