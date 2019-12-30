// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// Purpose: Helper APIs for metaswitch HALS stub programming 

#include "nic/metaswitch/stubs/mgmt/pds_ms_mgmt_utils.hpp"
#include "hals_mib.h"

namespace pds_ms {

// Fill halsEntTable: AMB_STUBS_HALS_ENT
NBB_VOID
pds_ms_fill_amb_hals_ent (AMB_GEN_IPS *mib_msg, pds_ms_config_t *conf)
{
    // Local variables
    NBB_ULONG           *oid = NULL; 
    AMB_STUBS_HALS_ENT  *data= NULL;

    NBB_TRC_ENTRY ("pds_ms_fill_amb_hals_ent");

    // Get oid and data offset 
    oid     = (NBB_ULONG *)((NBB_BYTE *)mib_msg + mib_msg->oid_offset);
    data    = (AMB_STUBS_HALS_ENT *)((NBB_BYTE *)mib_msg + mib_msg->data_offset); 

    // Set all fields absent
    AMB_SET_ALL_FIELDS_NOT_PRESENT (mib_msg);

    // Set OID len and family
    oid[0] = AMB_HALS_ENT_OID_LEN;
    oid[1] = AMB_FAM_STUBS_HALS_ENT;

    // Set all incoming fields
    oid[2]              = conf->entity_index; // Didn't find #define  OID index
    data->entity_index  = conf->entity_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_HALS_ENT_ENTITY_INDEX);

    data->row_status = conf->row_status;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_HALS_ENT_ROW_STATUS);

    if (conf->row_status != AMB_ROW_DESTROY)
    {
        NBB_TRC_FLOW((NBB_FORMAT "Not destroying HALS Ent: fill in fields"));
        data->stateful = conf->stateful;
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_HALS_ENT_STATEFUL);
    }

    NBB_TRC_EXIT ();
    return;
}

NBB_VOID
pds_ms_row_update_hals (pds_ms_config_t *conf)
{
    NBB_TRC_ENTRY ("pds_ms_row_update_hals");

    // Set params
    conf->oid_len       = AMB_HALS_ENT_OID_LEN;
    conf->data_len      = sizeof (AMB_STUBS_HALS_ENT);

    // Convert to row_update and send
    pds_ms_ctm_send_row_update_common (conf, pds_ms_fill_amb_hals_ent); 

    NBB_TRC_EXIT();
    return;
}

NBB_VOID
pds_ms_hals_stub_create (pds_ms_config_t *conf)
{
    NBB_TRC_ENTRY ("pds_ms_hals_stub_create");

    // halsEntTable
    conf->entity_index  = PDS_MS_HALS_ENT_INDEX;
    conf->stateful      = AMB_FALSE;
    pds_ms_row_update_hals (conf);

    NBB_TRC_EXIT();
    return;
}
}
