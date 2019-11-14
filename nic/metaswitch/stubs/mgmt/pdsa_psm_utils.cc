// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// Purpose: Helper APIs for metaswitch PSM stub programming 

#include "nic/metaswitch/stubs/mgmt/pdsa_mgmt_utils.hpp"
#include "psm_mgmt_if.h"

// Fill psmEntTable: AMB_PSM_ENT
NBB_VOID
pdsa_fill_amb_psm_ent (AMB_GEN_IPS *mib_msg, pdsa_config_t *conf)
{
    // Local variables
    NBB_ULONG   *oid = NULL; 
    AMB_PSM_ENT *data= NULL;

    NBB_TRC_ENTRY ("pdsa_fill_amb_psm_ent");

    // Get oid and data offset 
    oid     = (NBB_ULONG *)((NBB_BYTE *)mib_msg + mib_msg->oid_offset);
    data    = (AMB_PSM_ENT *)((NBB_BYTE *)mib_msg + mib_msg->data_offset); 

    // Set all fields absent
    AMB_SET_ALL_FIELDS_NOT_PRESENT (mib_msg);

    // Set OID len and family
    oid[0] = AMB_PSM_ENT_OID_LEN;
    oid[1] = AMB_FAM_PSM_ENT;

    // Set all incoming fields
    oid[2]              = conf->entity_index;
    data->entity_index  = conf->entity_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_PSM_ENT_ENTITY_INDEX);

    data->row_status    = conf->row_status;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_PSM_ENT_ROW_STATUS);

    NBB_TRC_EXIT ();
    return;
}

// Fill psmMjTable: AMB_PSM_MJ
NBB_VOID
pdsa_fill_amb_psm_mj (AMB_GEN_IPS *mib_msg, pdsa_config_t *conf)
{ 
    // Local variables
    NBB_ULONG   *oid = NULL; 
    AMB_PSM_MJ  *data= NULL;

    NBB_TRC_ENTRY ("pdsa_fill_amb_psm_mj");

    // Get oid and data offset 
    oid     = (NBB_ULONG *)((NBB_BYTE *)mib_msg + mib_msg->oid_offset);
    data    = (AMB_PSM_MJ *)((NBB_BYTE *)mib_msg + mib_msg->data_offset); 

    // Set all fields absent
    AMB_SET_ALL_FIELDS_NOT_PRESENT (mib_msg);

    // Set OID len and family
    oid[0] = AMB_PSM_MJ_OID_LEN;
    oid[1] = AMB_FAM_PSM_MJ;

    // Set all incoming fields
    oid[2]                  = conf->entity_index;
    data->psm_entity_index  = conf->entity_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_PSM_MJ_ENTITY_INDEX);

    oid[3]              = conf->interface_id;
    data->interface_id  = conf->interface_id;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_PSM_MJ_INTERFACE);

    oid[4]              = conf->partner_type;
    data->partner_type  = conf->partner_type;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_PSM_MJ_PARTNER_TYPE);

    oid[5]              = conf->partner_index;
    data->partner_index = conf->partner_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_PSM_MJ_PARTNER_INDEX);

    oid[6]              = conf->sub_index;
    data->sub_index     = conf->sub_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_PSM_MJ_SUB_INDEX);

    data->row_status    = conf->row_status;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_PSM_MJ_ROW_STATUS);

    NBB_TRC_EXIT ();
    return;
}

NBB_VOID
pdsa_test_row_update_psm (pdsa_config_t *conf)
{
    NBB_TRC_ENTRY ("pdsa_test_row_update_psm");

    // Set params
    conf->oid_len       = AMB_PSM_ENT_OID_LEN;
    conf->data_len      = sizeof (AMB_PSM_ENT);
    conf->entity_index  = 1;
    conf->row_status    = AMB_ROW_ACTIVE;

    // Convert to row_update and send
    pdsa_ctm_send_row_update_common (conf, pdsa_fill_amb_psm_ent); 

    NBB_TRC_EXIT();
    return;
}

NBB_VOID
pdsa_test_row_update_psm_mj (pdsa_config_t  *conf,
                             NBB_ULONG      interface_id,
                             NBB_ULONG      partner_type,
                             NBB_ULONG      partner_index,
                             NBB_ULONG      sub_index)
{
    NBB_TRC_ENTRY ("pdsa_test_row_update_ftm_mj");

    // Set params
    conf->oid_len       = AMB_PSM_MJ_OID_LEN;
    conf->data_len      = sizeof (AMB_PSM_MJ);
    conf->entity_index  = 1;
    conf->row_status    = AMB_ROW_ACTIVE;
    conf->interface_id  = interface_id;
    conf->partner_type  = partner_type;
    conf->partner_index = partner_index;
    conf->sub_index     = sub_index;

    // Convert to row_update and send
    pdsa_ctm_send_row_update_common (conf, pdsa_fill_amb_psm_mj); 

    NBB_TRC_EXIT();
    return;
}
