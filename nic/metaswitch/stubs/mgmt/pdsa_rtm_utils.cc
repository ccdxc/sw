// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// Purpose: Helper APIs for metaswitch RTM component

#include "nic/metaswitch/stubs/mgmt/pdsa_mgmt_utils.hpp"
#include "qc0rtmib.h"

// Fill rtmEntityTable: AMB_CIPR_RTM_ENTITY 
NBB_VOID 
pdsa_fill_amb_cipr_rtm (AMB_GEN_IPS *mib_msg, pdsa_config_t *conf)
{
    // Local variables
    NBB_ULONG           *oid = NULL; 
    AMB_CIPR_RTM_ENTITY *data= NULL;

    NBB_TRC_ENTRY ("pdsa_fill_amb_cipr_rtm");

    // Get oid and data offset 
    oid     = (NBB_ULONG *)((NBB_BYTE *)mib_msg + mib_msg->oid_offset);
    data    = (AMB_CIPR_RTM_ENTITY *)((NBB_BYTE *)mib_msg + mib_msg->data_offset); 

    // Set all fields absentt
    AMB_SET_ALL_FIELDS_NOT_PRESENT (mib_msg);

    // Set OID len and family
    oid[0] = AMB_QCR_ENT_OID_LEN;
    oid[1] = AMB_FAM_CIPR_RTM_ENTITY;

    // Set all incoming fields
    oid[AMB_QCR_ENT_FTE_INDEX_INDEX]    = conf->entity_index;
    data->fte_index                     = conf->entity_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_QCR_ENT_FTE_INDEX);

    data->row_status = conf->row_status;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_QCR_ENT_ROW_STATUS);

    if (conf->row_status != AMB_ROW_DESTROY)
    {
        NBB_TRC_FLOW ((NBB_FORMAT "Not destroying DC-RTM: fill in fields"));
        data->admin_stat = conf->admin_status;
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_QCR_ENT_ADMIN_STAT);

        data->addr_family = conf->addr_family;
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_QCR_ENT_ADDR_FAM);

        data->i3_index = conf->i3_index;
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_QCR_ENT_I3_INDEX);
    }

    NBB_TRC_EXIT();
    return;
} 

// Fill ftsEntityTable: AMB_CIPR_FTS_ENTITY 
NBB_VOID 
pdsa_fill_amb_cipr_fts (AMB_GEN_IPS *mib_msg, pdsa_config_t *conf)
{
    // Local variables
    NBB_ULONG           *oid = NULL; 
    AMB_CIPR_FTS_ENTITY *data= NULL;

    NBB_TRC_ENTRY ("pdsa_fill_amb_cipr_fts");

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


// Fill rtmMjTable: AMB_CIPR_RTM_MJ
NBB_VOID
pdsa_fill_amb_cipr_rtm_mj (AMB_GEN_IPS *mib_msg, pdsa_config_t *conf)
{
    // Local variables
    NBB_ULONG       *oid = NULL; 
    AMB_CIPR_RTM_MJ *data= NULL;

    NBB_TRC_ENTRY ("sms_fill_amb_cipr_rtm_mj");

    // Get oid and data offset 
    oid     = (NBB_ULONG *)((NBB_BYTE *)mib_msg + mib_msg->oid_offset);
    data    = (AMB_CIPR_RTM_MJ *)((NBB_BYTE *)mib_msg + mib_msg->data_offset); 

    // Set all fields absent
    AMB_SET_ALL_FIELDS_NOT_PRESENT (mib_msg);

    // Set OID len and family
    oid[0] = AMB_QRPM_MJ_OID_LEN;
    oid[1] = AMB_FAM_CIPR_RTM_MJ;

    // Set all incoming fields
    oid[AMB_QRPM_MJ_RTM_FTE_INDEX_INDEX]    = conf->entity_index;
    data->rtm_fte_index                     = conf->entity_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_QRPM_MJ_RTM_FTE_INDEX);

    oid[AMB_QRPM_MJ_SLAVE_FTE_ID_INDEX]     = conf->slave_entity_index;
    data->slave_fte_id                      = conf->slave_entity_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_QRPM_MJ_SLAVE_FTE_ID);

    oid[AMB_QRPM_MJ_SLAVE_TYPE_INDEX]       = conf->slave_type;
    data->slave_type                        = conf->slave_type;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_QRPM_MJ_SLAVE_TYPE);

    data->row_status = conf->row_status;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_QRPM_MJ_ROW_STATUS);

    if (conf->row_status != AMB_ROW_DESTROY)
    {
        NBB_TRC_FLOW ((NBB_FORMAT "Not DC-RTM: fill in fields"));
        data->admin_status = conf->admin_status;
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_QRPM_MJ_ADMIN_STATUS);

    }

    NBB_TRC_EXIT();
    return;
} 

// Fill rtmRedistTable: AMB_CIPR_RTM_REDIST 
NBB_VOID 
pdsa_fill_amb_cipr_rtm_redist (AMB_GEN_IPS *mib_msg, pdsa_config_t *conf)
{
    // Local variables
    NBB_ULONG           *oid = NULL; 
    AMB_CIPR_RTM_REDIST *data= NULL;

    NBB_TRC_ENTRY ("pdsa_fill_amb_cipr_rtm_redist");

    // Get oid and data offset 
    oid     = (NBB_ULONG *)((NBB_BYTE *)mib_msg + mib_msg->oid_offset);
    data    = (AMB_CIPR_RTM_REDIST *)((NBB_BYTE *)mib_msg + mib_msg->data_offset); 

    // Set all fields absentt
    AMB_SET_ALL_FIELDS_NOT_PRESENT (mib_msg);

    // Set OID len and family
    oid[0] = AMB_QCR_RDS_OID_LEN;
    oid[1] = AMB_FAM_CIPR_RTM_REDIST;

    // Set all incoming fields
    oid[AMB_QCR_RDS_FTE_INDEX_INDEX]    = conf->entity_index;
    data->fte_index                     = conf->entity_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_QCR_RDS_FTE_INDEX);

    oid[AMB_QCR_RDS_ENTRY_ID_INDEX]    = 1;
    data->entry_id                     = 1;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_QCR_RDS_ENTRY_ID);

    data->row_status = conf->row_status;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_QCR_RDS_ROW_STATUS);

    if (conf->row_status != AMB_ROW_DESTROY)
    {
        NBB_TRC_FLOW ((NBB_FORMAT "Not destroying RTM Redistribute: fill in fields"));
        data->admin_stat = conf->admin_status;
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_QCR_RDS_ADMIN_STAT);

        data->info_src = ATG_QC_PROT_CONNECTED;
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_QCR_RDS_INFO_SRC);

        data->info_dest = ATG_QC_PROT_BGP;
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_QCR_RDS_INFO_DEST);

        data->redist_flag = AMB_TRUE;
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_QCR_RDS_REDIST_FLAG);
    }

    NBB_TRC_EXIT();
    return;
} 

NBB_VOID
pdsa_test_row_update_rtm (pdsa_config_t *conf, NBB_LONG  admin_status)
{
    NBB_TRC_ENTRY ("pdsa_test_row_update_rtm");

    // Set params
    conf->oid_len       = AMB_QCR_ENT_OID_LEN;
    conf->data_len      = sizeof (AMB_CIPR_RTM_ENTITY);
    conf->entity_index  = 1;
    conf->row_status    = AMB_ROW_ACTIVE;
    conf->admin_status  = admin_status;
    conf->addr_family   = AMB_INETWK_ADDR_TYPE_IPV4;
    conf->i3_index      = 1;

    // Convert to row_update and send
    pdsa_ctm_send_row_update_common (conf, pdsa_fill_amb_cipr_rtm); 

    NBB_TRC_EXIT();
    return;
}

NBB_VOID
pdsa_test_row_update_fts (pdsa_config_t *conf, NBB_LONG  admin_status)
{
    NBB_TRC_ENTRY ("pdsa_test_row_update_fts");

    // Set params
    conf->oid_len       = AMB_FTS_ENT_OID_LEN;
    conf->data_len      = sizeof (AMB_CIPR_FTS_ENTITY);
    conf->entity_index  = 1;
    conf->row_status    = AMB_ROW_ACTIVE;
    conf->admin_status  = admin_status;

    // Convert to row_update and send
    pdsa_ctm_send_row_update_common (conf, pdsa_fill_amb_cipr_fts); 

    NBB_TRC_EXIT();
    return;
}

NBB_VOID
pdsa_test_row_update_rtm_mj (pdsa_config_t *conf, NBB_LONG slave_type)
{
    NBB_TRC_ENTRY ("pdsa_test_row_update_rtm_mj");
    
    // Set params
    conf->oid_len               = AMB_QRPM_MJ_OID_LEN;
    conf->data_len              = sizeof (AMB_CIPR_RTM_MJ);
    conf->entity_index          = 1;
    conf->row_status            = AMB_ROW_ACTIVE;
    conf->slave_entity_index    = 1;
    conf->slave_type            = slave_type;
    conf->admin_status          = AMB_ADMIN_STATUS_UP;


    // Convert to row_update and send
    pdsa_ctm_send_row_update_common (conf, pdsa_fill_amb_cipr_rtm_mj); 

    NBB_TRC_EXIT();
    return;
}

NBB_VOID pdsa_rtm_redis_connected (pdsa_config_t *conf)
{
    conf->oid_len               = AMB_QCR_RDS_OID_LEN;
    conf->data_len              = sizeof (AMB_CIPR_RTM_REDIST);
    conf->entity_index          = 1;
    conf->row_status            = AMB_ROW_ACTIVE;
    conf->admin_status          = AMB_ADMIN_STATUS_UP;

    pdsa_ctm_send_row_update_common (conf, pdsa_fill_amb_cipr_rtm_redist); 
    return;
}

