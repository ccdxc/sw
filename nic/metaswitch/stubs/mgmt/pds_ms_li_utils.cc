// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// Purpose: Helper APIs for metaswitch LI stub programming 

#include "nic/metaswitch/stubs/mgmt/pds_ms_mgmt_utils.hpp"
#include "li_mgmt_if.h"

namespace pds_ms_stub {

// Fill liEntTable: AMB_STUBS_LI_ENT
static NBB_VOID
pds_ms_fill_amb_li_ent (AMB_GEN_IPS *mib_msg, pds_ms_config_t *conf)
{ 
    // Local variables
    NBB_ULONG           *oid = NULL; 
    AMB_STUBS_LI_ENT    *data= NULL;

    NBB_TRC_ENTRY ("pds_ms_fill_amb_li_ent");

    // Get oid and data offset 
    oid     = (NBB_ULONG *)((NBB_BYTE *)mib_msg + mib_msg->oid_offset);
    data    = (AMB_STUBS_LI_ENT *)((NBB_BYTE *)mib_msg + mib_msg->data_offset); 

    // Set all fields absent
    AMB_SET_ALL_FIELDS_NOT_PRESENT (mib_msg);

    // Set OID len and family
    oid[0] = AMB_LI_ENT_OID_LEN;
    oid[1] = AMB_FAM_STUBS_LI_ENT;

    // Set all incoming fields
    data->entity_index              = conf->entity_index;
    oid[AMB_LI_ENT_ENT_IX_INDEX]    = conf->entity_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_LI_ENT_ENTITY_INDEX);

    data->row_status = conf->row_status;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_LI_ENT_ROW_STATUS);

    if (data->row_status != AMB_ROW_DESTROY)
    {
        NBB_TRC_FLOW((NBB_FORMAT "Not destroying LI Ent: fill in fields"));
        data->stateful = conf->stateful;
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_LI_ENT_STATEFUL);
    }
    NBB_TRC_EXIT ();
    return;
}

// Fill liMjTable: AMB_STUBS_LI_MJ
static NBB_VOID
pds_ms_fill_amb_li_mj (AMB_GEN_IPS *mib_msg, pds_ms_config_t *conf)
{ 
    // Local variables
    NBB_ULONG       *oid = NULL; 
    AMB_STUBS_LI_MJ *data= NULL;

    NBB_TRC_ENTRY ("pds_ms_fill_amb_li_mj");

    // Get oid and data offset 
    oid     = (NBB_ULONG *)((NBB_BYTE *)mib_msg + mib_msg->oid_offset);
    data    = (AMB_STUBS_LI_MJ *)((NBB_BYTE *)mib_msg + mib_msg->data_offset); 

    // Set all fields absent
    AMB_SET_ALL_FIELDS_NOT_PRESENT (mib_msg);

    // Set OID len and family
    oid[0] = AMB_LI_MJ_OID_LEN;
    oid[1] = AMB_FAM_STUBS_LI_MJ;

    // Set all incoming fields
    data->li_entity_index           = conf->entity_index;
    oid[AMB_LI_MJ_APPL_INDEX_INDEX] = conf->entity_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_LI_MJ_ENTITY_INDEX);

    data->interface_id              = conf->interface_id;
    oid[AMB_LI_MJ_IF_TYPE_INDEX]    = conf->interface_id;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_LI_MJ_INTERFACE);

    data->partner_type                  = conf->partner_type;
    oid[AMB_LI_MJ_PARTNER_TYPE_INDEX]   = conf->partner_type;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_LI_MJ_PARTNER_TYPE);

    data->partner_index                 = conf->partner_index;
    oid[AMB_LI_MJ_PARTNER_INDEX_INDEX]  = conf->partner_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_LI_MJ_PARTNER_INDEX);

    data->sub_index                 = conf->sub_index;
    oid[AMB_LI_MJ_SUB_INDEX_INDEX]  = conf->sub_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_LI_MJ_SUB_INDEX);

    data->row_status = conf->row_status;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_LI_MJ_ROW_STATUS);

    NBB_TRC_EXIT ();
    return;
}


static NBB_VOID
pds_ms_row_update_li (pds_ms_config_t *conf)
{
    NBB_TRC_ENTRY ("pds_ms_row_update_li");

    // Set params
    conf->oid_len       = AMB_LI_ENT_OID_LEN;
    conf->data_len      = sizeof (AMB_STUBS_LI_ENT);

    // Convert to row_update and send
    pds_ms_ctm_send_row_update_common (conf, pds_ms_fill_amb_li_ent); 

    NBB_TRC_EXIT();
    return;
}

static NBB_VOID
pds_ms_row_update_li_mj (pds_ms_config_t   *conf)
{
    NBB_TRC_ENTRY ("pds_ms_row_update_li_mj");

    // Set params
    conf->oid_len       = AMB_LI_MJ_OID_LEN;
    conf->data_len      = sizeof (AMB_STUBS_LI_MJ);

    // Convert to row_update and send
    pds_ms_ctm_send_row_update_common (conf, pds_ms_fill_amb_li_mj); 

    NBB_TRC_EXIT();
    return;
}

NBB_VOID
pds_ms_li_stub_create (pds_ms_config_t *conf)
{
    NBB_TRC_ENTRY ("pds_ms_li_stub_create");

    // liEntTable
    conf->entity_index   = PDS_MS_LI_ENT_INDEX;
    conf->stateful       = AMB_FALSE;
    pds_ms_row_update_li (conf);

    // liMjTable
    conf->entity_index   = PDS_MS_LI_ENT_INDEX;
    conf->interface_id   = AMB_LI_IF_ATG_FRI;
    conf->partner_type   = AMB_LI_MJ_PARTNER_LIM;
    conf->partner_index  = 1;
    conf->sub_index      = 0;
    pds_ms_row_update_li_mj (conf);

    NBB_TRC_EXIT();
    return;
}
}
