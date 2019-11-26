// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// Purpose: Helper APIs for metaswitch Sock stub programming 

#include "nic/metaswitch/stubs/mgmt/pdsa_mgmt_utils.hpp"
#include "sck0mib.h"

namespace pdsa_stub {

// Fill sckTable: AMB_STUBS_SCK
NBB_VOID
pdsa_fill_amb_sck (AMB_GEN_IPS *mib_msg, pdsa_config_t *conf)
{
    // Local variables
    NBB_ULONG       *oid = NULL; 
    AMB_STUBS_SCK   *data= NULL;

    NBB_TRC_ENTRY ("pdsa_fill_amb_sck");

    // Get oid and data offset 
    oid     = (NBB_ULONG *)((NBB_BYTE *)mib_msg + mib_msg->oid_offset);
    data    = (AMB_STUBS_SCK *)((NBB_BYTE *)mib_msg + mib_msg->data_offset); 

    // Set all fields absent
    AMB_SET_ALL_FIELDS_NOT_PRESENT (mib_msg);
    
    // Set OID len and family
    oid[0] = AMB_STUBS_SCK_OID_LEN;
    oid[1] = AMB_FAM_STUBS_SCK;

    // Set all incoming fields
    oid[AMB_STUBS_SCK_INDEX_INDEX]  = conf->entity_index;
    data->index                     = conf->entity_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_STUBS_SCK_INDEX);

    data->row_status = conf->row_status;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_STUBS_SCK_ROW_STATUS);

    NBB_TRC_EXIT ();
    return;
}


NBB_VOID
pdsa_row_update_sck (pdsa_config_t *conf)
{
    NBB_TRC_ENTRY ("pdsa_row_update_sck");

    // Set params
    conf->oid_len       = AMB_STUBS_SCK_OID_LEN;
    conf->data_len      = sizeof (AMB_STUBS_SCK);
    conf->entity_index  = 1;

    // Convert to row_update and send
    pdsa_ctm_send_row_update_common (conf, pdsa_fill_amb_sck); 

    NBB_TRC_EXIT();
    return;
}
}
