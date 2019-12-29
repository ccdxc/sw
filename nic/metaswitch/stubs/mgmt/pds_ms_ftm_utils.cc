// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//Purpose: Helper APIs for metaswitch FTM stub programming 

#include "nic/metaswitch/stubs/mgmt/pds_ms_mgmt_utils.hpp"
#include "ftm_mgmt_if.h"

namespace pds_ms_stub {

// Fill ftmEntTable: AMB_FTM_ENT
NBB_VOID
pds_ms_fill_amb_ftm_ent (AMB_GEN_IPS *mib_msg,  pds_ms_config_t *conf)
{
    // Local variables
    NBB_ULONG       *oid = NULL; 
    AMB_FTM_ENT     *data= NULL;

    NBB_TRC_ENTRY ("pds_ms_fill_amb_ftm_ent");

    // Get oid and data offset 
    oid     = (NBB_ULONG *)((NBB_BYTE *)mib_msg + mib_msg->oid_offset);
    data    = (AMB_FTM_ENT *)((NBB_BYTE *)mib_msg + mib_msg->data_offset); 

    // Set all fields absent
    AMB_SET_ALL_FIELDS_NOT_PRESENT (mib_msg);

    // Set OID len and family
    oid[0] = AMB_FTM_ENT_OID_LEN;
    oid[1] = AMB_FAM_FTM_ENT;

    // Set all incoming fields
    oid[2]              = conf->entity_index;
    data->entity_index  = conf->entity_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_FTM_ENT_ENTITY_INDEX);

    data->row_status = conf->row_status;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_FTM_ENT_ROW_STATUS);

    NBB_TRC_EXIT ();
    return;
}

// Fill ftmMjTable: AMB_FTM_MJ
NBB_VOID
pds_ms_fill_amb_ftm_mj (AMB_GEN_IPS *mib_msg, pds_ms_config_t *conf)
{ 
    // Local variables
    NBB_ULONG   *oid = NULL; 
    AMB_FTM_MJ  *data= NULL;

    NBB_TRC_ENTRY ("pds_ms_fill_amb_ftm_mj");

    // Get oid and data offset 
    oid     = (NBB_ULONG *)((NBB_BYTE *)mib_msg + mib_msg->oid_offset);
    data    = (AMB_FTM_MJ *)((NBB_BYTE *)mib_msg + mib_msg->data_offset); 

    // Set all fields absent
    AMB_SET_ALL_FIELDS_NOT_PRESENT (mib_msg);

    // Set OID len and family
    oid[0] = AMB_FTM_MJ_OID_LEN;
    oid[1] = AMB_FAM_FTM_MJ;

    // Set all incoming fields
    oid[2]                  = conf->entity_index; // Didn't find OID indices for FTM MJ
    data->ftm_entity_index  = conf->entity_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_FTM_MJ_ENTITY_INDEX);

    oid[3]             = conf->interface_id;
    data->interface_id = conf->interface_id;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_FTM_MJ_INTERFACE);

    oid[4]              = conf->partner_type;
    data->partner_type  = conf->partner_type;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_FTM_MJ_PARTNER_TYPE);

    oid[5]              = conf->partner_index;
    data->partner_index = conf->partner_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_FTM_MJ_PARTNER_INDEX);

    oid[6]          = conf->sub_index;
    data->sub_index = conf->sub_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_FTM_MJ_SUB_INDEX);

    data->row_status = conf->row_status;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_FTM_MJ_ROW_STATUS);

    NBB_TRC_EXIT ();
    return;
}

NBB_VOID
pds_ms_row_update_ftm (pds_ms_config_t *conf)
{
    NBB_TRC_ENTRY ("pds_ms_row_update_ftm");
    
    // Set params
    conf->oid_len       = AMB_FTM_ENT_OID_LEN;
    conf->data_len      = sizeof (AMB_FTM_ENT);

    // Convert to row_update and send
    pds_ms_ctm_send_row_update_common (conf, pds_ms_fill_amb_ftm_ent); 

    NBB_TRC_EXIT();
    return;
}

NBB_VOID
pds_ms_row_update_ftm_mj (pds_ms_config_t  *conf)
{
    NBB_TRC_ENTRY ("pds_ms_row_update_ftm_mj");

    // Set params
    conf->oid_len       = AMB_FTM_MJ_OID_LEN;
    conf->data_len      = sizeof (AMB_FTM_MJ);

    // Convert to row_update and send
    pds_ms_ctm_send_row_update_common (conf, pds_ms_fill_amb_ftm_mj); 

    NBB_TRC_EXIT();
    return;
}

NBB_VOID
pds_ms_ftm_create (pds_ms_config_t *conf)
{
    NBB_TRC_ENTRY ("pds_ms_ftm_create");

    // ftmEntTable
    conf->entity_index   = PDS_MS_FTM_ENT_INDEX;
    pds_ms_row_update_ftm (conf);

    // ftmMjTable - ROPI
    conf->interface_id   = AMB_FTM_IF_ATG_ROPI;
    conf->partner_type   = AMB_FTM_MJ_PARTNER_HALS;
    conf->partner_index  = 1;
    conf->sub_index      = 0;
    pds_ms_row_update_ftm_mj (conf);

    // ftmMjTable - PRI
    conf->interface_id   = AMB_FTM_IF_ATG_PRI;
    conf->partner_type   = AMB_FTM_MJ_PARTNER_PSM;
    conf->partner_index  = 1;
    conf->sub_index      = 0;
    pds_ms_row_update_ftm_mj (conf);

    NBB_TRC_EXIT();
    return;
}
}
