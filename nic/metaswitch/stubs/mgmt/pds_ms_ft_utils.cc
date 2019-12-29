// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// Purpose: Helper APIs for metaswitch RTM component

#include "nic/metaswitch/stubs/mgmt/pds_ms_mgmt_utils.hpp"

namespace pds_ms_stub {

// Fill ftsEntityTable: AMB_CIPR_FTS_ENTITY 
NBB_VOID 
pds_ms_fill_amb_cipr_fts (AMB_GEN_IPS *mib_msg, pds_ms_config_t *conf)
{
    // Local variables
    NBB_ULONG           *oid = NULL; 
    AMB_CIPR_FTS_ENTITY *data= NULL;

    NBB_TRC_ENTRY ("pds_ms_fill_amb_cipr_fts");

    // Get oid and data offset 
    oid     = (NBB_ULONG *)((NBB_BYTE *)mib_msg + mib_msg->oid_offset);
    data    = (AMB_CIPR_FTS_ENTITY *)((NBB_BYTE *)mib_msg + mib_msg->data_offset); 

    // Set all fields absentt
    AMB_SET_ALL_FIELDS_NOT_PRESENT (mib_msg);

    // Set OID len and family
    oid[0] = AMB_FTS_ENT_OID_LEN;
    oid[1] = AMB_FAM_CIPR_FTS_ENTITY;

    // Set all incoming fields
    oid[AMB_FTS_ENT_INDEX_INDEX]    = conf->entity_index;
    data->index                     = conf->entity_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_FTS_ENT_INDEX);

    data->row_status = conf->row_status;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_FTS_ENT_ROW_STATUS);

    if (conf->row_status != AMB_ROW_DESTROY)
    {
        NBB_TRC_FLOW ((NBB_FORMAT "Not destroying DC-FTS: fill in fields"));
        data->admin_status = conf->admin_status;
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_FTS_ENT_ADMIN_STATUS);

        // No route state needed in FT stub
        data->store_route_state = AMB_FALSE;
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_FTS_ENT_STORE_STATE);

        // Same FT Stub Can connect to both v4 and v6 RTM instances belonging 
        // to the same VRF
        data->num_ari_partners = 2;
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_FTS_ENT_NUM_ARI_PARTNER);
    }

    NBB_TRC_EXIT();
    return;
} 

NBB_VOID
pds_ms_row_update_fts (pds_ms_config_t *conf)
{
    NBB_TRC_ENTRY ("pds_ms_row_update_fts");

    // Set params
    conf->oid_len       = AMB_FTS_ENT_OID_LEN;
    conf->data_len      = sizeof (AMB_CIPR_FTS_ENTITY);

    // Convert to row_update and send
    pds_ms_ctm_send_row_update_common (conf, pds_ms_fill_amb_cipr_fts); 

    NBB_TRC_EXIT();
    return;
}

NBB_VOID
pds_ms_ft_stub_create (pds_ms_config_t *conf)
{
    NBB_TRC_ENTRY ("pds_ms_ft_stub_create");

    // FT Stub - ftsEntityTable
    conf->entity_index  = PDS_MS_FT_ENT_INDEX;
    conf->admin_status  = AMB_ADMIN_STATUS_UP;
    pds_ms_row_update_fts (conf);

    NBB_TRC_EXIT();
    return;
}
}
