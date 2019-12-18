// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// Purpose: Helper APIs for metaswitch LIM stub programming 

#include "nic/metaswitch/stubs/mgmt/pdsa_mgmt_utils.hpp"
#include "evpn_prod.h"
#include "lim_mgmt_if.h"

namespace pdsa_stub {

// Fill limEntTable: AMB_LIM_ENT
NBB_VOID
pdsa_fill_amb_lim_ent (AMB_GEN_IPS *mib_msg, pdsa_config_t *conf)
{ 
    // Local variables
    NBB_ULONG   *oid = NULL; 
    AMB_LIM_ENT *data= NULL;

    NBB_TRC_ENTRY ("pdsa_fill_amb_lim_ent");

    // Get oid and data offset 
    oid     = (NBB_ULONG *)((NBB_BYTE *)mib_msg + mib_msg->oid_offset);
    data    = (AMB_LIM_ENT *)((NBB_BYTE *)mib_msg + mib_msg->data_offset); 

    // Set all fields absent
    AMB_SET_ALL_FIELDS_NOT_PRESENT (mib_msg);

    // Set OID len and family
    oid[0] = AMB_LIM_ENT_OID_LEN;
    oid[1] = AMB_FAM_LIM_ENT;

    // Set all incoming fields
    data->entity_index              = conf->entity_index;
    oid[AMB_LIM_ENT_ENT_IX_INDEX]   = conf->entity_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_LIM_ENT_ENTITY_INDEX);

    data->row_status = conf->row_status;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_LIM_ENT_ROW_STATUS);

    NBB_TRC_EXIT ();
    return;
}

// Fill limMjTable: AMB_LIM_MJ
NBB_VOID
pdsa_fill_amb_lim_mj (AMB_GEN_IPS *mib_msg, pdsa_config_t *conf)
{ 
    // Local variables
    NBB_ULONG   *oid = NULL; 
    AMB_LIM_MJ  *data= NULL;

    NBB_TRC_ENTRY ("pdsa_fill_amb_lim_mj");

    // Get oid and data offset 
    oid     = (NBB_ULONG *)((NBB_BYTE *)mib_msg + mib_msg->oid_offset);
    data    = (AMB_LIM_MJ *)((NBB_BYTE *)mib_msg + mib_msg->data_offset); 

    // Set all fields absent
    AMB_SET_ALL_FIELDS_NOT_PRESENT (mib_msg);

    // Set OID len and family
    oid[0] = AMB_LIM_MJ_OID_LEN;
    oid[1] = AMB_FAM_LIM_MJ;

    // Set all incoming fields
    data->lim_entity_index              = conf->entity_index;
    oid[AMB_LIM_MJ_APPL_INDEX_INDEX]    = conf->entity_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_LIM_MJ_ENTITY_INDEX);

    data->interface_id                  = conf->interface_id;
    oid[AMB_LIM_MJ_IF_TYPE_INDEX]       = conf->interface_id;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_LIM_MJ_INTERFACE);

    data->partner_type                  = conf->partner_type;
    oid[AMB_LIM_MJ_PARTNER_TYPE_INDEX]  = conf->partner_type;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_LIM_MJ_PARTNER_TYPE);

    data->partner_index                 = conf->partner_index;
    oid[AMB_LIM_MJ_PARTNER_INDEX_INDEX] = conf->partner_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_LIM_MJ_PARTNER_INDEX);

    data->sub_index                     = conf->sub_index;
    oid[AMB_LIM_MJ_SUB_INDEX_INDEX]     = conf->sub_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_LIM_MJ_SUB_INDEX);

    data->row_status = conf->row_status;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_LIM_MJ_ROW_STATUS);

    NBB_TRC_EXIT ();
    return;
}

NBB_VOID
pdsa_row_update_lim (pdsa_config_t *conf)
{
    NBB_TRC_ENTRY ("pdsa_row_update_lim");

    // Set params
    conf->oid_len       = AMB_LIM_ENT_OID_LEN;
    conf->data_len      = sizeof (AMB_LIM_ENT);

    // Convert to row_update and send
    pdsa_ctm_send_row_update_common (conf, pdsa_fill_amb_lim_ent); 

    NBB_TRC_EXIT();
    return;
}

NBB_VOID
pdsa_row_update_lim_mj (pdsa_config_t  *conf)
{
    NBB_TRC_ENTRY ("pdsa_row_update_lim_mj");

    // Set params
    conf->oid_len       = AMB_LIM_ENT_OID_LEN;
    conf->data_len      = sizeof (AMB_LIM_ENT);
    
    // Convert to row_update and send
    pdsa_ctm_send_row_update_common (conf, pdsa_fill_amb_lim_mj); 

    NBB_TRC_EXIT();
    return;
}

NBB_VOID
pdsa_lim_create (pdsa_config_t *conf)
{
    NBB_TRC_ENTRY ("pdsa_lim_create");

    // limEntTable
    conf->entity_index  = PDSA_LIM_ENT_INDEX;
    pdsa_row_update_lim (conf);

    // limMjTable - LIPI
    conf->interface_id   = AMB_LIM_IF_ATG_LIPI;
    conf->partner_type   = AMB_LIM_MJ_PARTNER_LI;
    conf->partner_index  = 1;
    conf->sub_index      = 0;
    pdsa_row_update_lim_mj (conf);

    // limMjTable - SMI
    conf->interface_id   = AMB_LIM_IF_ATG_SMI;
    conf->partner_type   = AMB_LIM_MJ_PARTNER_SMI;
    conf->partner_index  = 1;
    conf->sub_index      = 0;
    pdsa_row_update_lim_mj (conf);

    // limMjTable - BDII to EVPN
    conf->interface_id   = AMB_LIM_IF_ATG_BDII;
    conf->partner_type   = AMB_LIM_MJ_PARTNER_EVPN;
    conf->partner_index  = 1;
    conf->sub_index      = 0;
    pdsa_row_update_lim_mj (conf);

    NBB_TRC_EXIT();
    return;
}
}
