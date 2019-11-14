// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// Purpose: Helper APIs for metaswitch NAR stub programming 

#include "nic/metaswitch/stubs/mgmt/pdsa_mgmt_utils.hpp"
#include "nar_mib.h"

// Fill narEntTable: AMB_STUBS_NAR_ENT
NBB_VOID
pdsa_fill_amb_nar_ent (AMB_GEN_IPS *mib_msg, pdsa_config_t *conf)
{
    // Local variables
    NBB_ULONG           *oid = NULL; 
    AMB_STUBS_NAR_ENT   *data= NULL;

    NBB_TRC_ENTRY ("pdsa_fill_amb_nar_ent");

    // Get oid and data offset 
    oid     = (NBB_ULONG *)((NBB_BYTE *)mib_msg + mib_msg->oid_offset);
    data    = (AMB_STUBS_NAR_ENT *)((NBB_BYTE *)mib_msg + mib_msg->data_offset); 

    // Set all fields absent
    AMB_SET_ALL_FIELDS_NOT_PRESENT (mib_msg);

    // Set OID len and family
    oid[0] = AMB_NAR_ENT_OID_LEN;
    oid[1] = AMB_FAM_STUBS_NAR_ENT;

    // Set all incoming fields
    oid[2]              = conf->entity_index;
    data->entity_index  = conf->entity_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_NAR_ENT_ENTITY_INDEX);

    data->row_status = conf->row_status;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_NAR_ENT_ROW_STATUS);

    NBB_TRC_EXIT ();
    return;
}

NBB_VOID
pdsa_test_row_update_nar (pdsa_config_t *conf)
{
    NBB_TRC_ENTRY ("pdsa_test_row_update_nar");

    // Set params
    conf->oid_len       = AMB_NAR_ENT_OID_LEN;
    conf->data_len      = sizeof (AMB_STUBS_NAR_ENT);
    conf->entity_index  = 1;
    conf->row_status    = AMB_ROW_ACTIVE;

    // Convert to row_update and send
    pdsa_ctm_send_row_update_common (conf, pdsa_fill_amb_nar_ent); 

    NBB_TRC_EXIT();
    return;
}
