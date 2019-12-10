// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//Purpose: Helper APIs for metaswitch EVPN component

#include "nic/metaswitch/stubs/mgmt/pdsa_mgmt_utils.hpp"
#include "evpn_mgmt_if.h"

namespace pdsa_stub {

// Fill evpnEntTable: AMB_EVPN_ENT 
NBB_VOID
pdsa_fill_amb_evpn_ent (AMB_GEN_IPS  *mib_msg, pdsa_config_t *conf)
{ 
    // Local variables
    NBB_ULONG       *oid = NULL; 
    AMB_EVPN_ENT    *data= NULL;

    NBB_TRC_ENTRY ("pdsa_fill_amb_evpn");

    // Get oid and data offset 
    oid     = (NBB_ULONG *)((NBB_BYTE *)mib_msg + mib_msg->oid_offset);
    data    = (AMB_EVPN_ENT *)((NBB_BYTE *)mib_msg + mib_msg->data_offset); 

    // Set all fields absent
    AMB_SET_ALL_FIELDS_NOT_PRESENT (mib_msg);

    // Set OID len and family
    oid[0] = AMB_EVPN_ENT_OID_LEN;
    oid[1] = AMB_FAM_EVPN_ENT;

    // Set all incoming fields
    oid[AMB_EVPN_ENT_ENTITY_INDEX_INDEX]    = conf->entity_index;
    data->entity_index                      = conf->entity_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_EVPN_ENT_ENTITY_INDEX);

    data->row_status = conf->row_status;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_EVPN_ENT_ROW_STATUS);

    NBB_TRC_EXIT ();
    return;
}

// Fill evpnMjTable:AMB_EVPN_MJ 
NBB_VOID
pdsa_fill_amb_evpn_mj (AMB_GEN_IPS  *mib_msg, pdsa_config_t *conf)
{ 
    // Local variables
    NBB_ULONG   *oid = NULL; 
    AMB_EVPN_MJ *data= NULL;

    NBB_TRC_ENTRY ("pdsa_fill_amb_evpn_mj");

    // Get oid and data offset 
    oid     = (NBB_ULONG *)((NBB_BYTE *)mib_msg + mib_msg->oid_offset);
    data    = (AMB_EVPN_MJ *)((NBB_BYTE *)mib_msg + mib_msg->data_offset); 

    // Set all fields absent
    AMB_SET_ALL_FIELDS_NOT_PRESENT (mib_msg);

    // Set OID len and family
    oid[0] = AMB_EVPN_MJ_OID_LEN;
    oid[1] = AMB_FAM_EVPN_MJ;

    // Set all incoming fields
    oid[AMB_EVPN_MJ_ENTITY_IX_INDEX]    = conf->entity_index;
    data->ent_entity_index              = conf->entity_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_EVPN_MJ_ENT_ENTITY_IX);

    oid[AMB_EVPN_MJ_INTERFACE_ID_INDEX] = conf->interface_id;
    data->interface_id                  = conf->interface_id;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_EVPN_MJ_INTERFACE_ID);

    oid[AMB_EVPN_MJ_PARTNER_TYPE_INDEX] = conf->partner_type;
    data->partner_type                  = conf->partner_type;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_EVPN_MJ_PARTNER_TYPE);

    oid[AMB_EVPN_MJ_PARTNER_INDEX_INDEX]    = conf->partner_index;
    data->partner_index                     = conf->partner_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_EVPN_MJ_PARTNER_INDEX);

    oid[AMB_EVPN_MJ_SUB_INDEX_INDEX]    = conf->sub_index;
    data->sub_index                     = conf->sub_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_EVPN_MJ_SUB_INDEX);

    data->row_status = conf->row_status;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_EVPN_MJ_ROW_STATUS);

    NBB_TRC_EXIT ();
    return;
}

// row_update for evpnEntTable
NBB_VOID
pdsa_row_update_evpn (pdsa_config_t *conf)
{
    NBB_TRC_ENTRY ("pdsa_row_update_evpn");

    // Set params
    conf->oid_len       = AMB_EVPN_ENT_OID_LEN;
    conf->data_len      = sizeof (AMB_EVPN_ENT);

    //pdsa_convert_long_to_pdsa_ipv4_addr (conf->g_node_a_ip, &conf->ip_addr);

    // Convert to row_update and send
    pdsa_ctm_send_row_update_common (conf, pdsa_fill_amb_evpn_ent); 

    NBB_TRC_EXIT();
    return;
}

NBB_VOID
pdsa_row_update_evpn_mj (pdsa_config_t *conf)
{
    NBB_TRC_ENTRY ("pdsa_row_update_evpn_mj");

    // Set params
    conf->oid_len       = AMB_EVPN_MJ_OID_LEN;
    conf->data_len      = sizeof (AMB_EVPN_MJ);

    // Convert to row_update and send
    pdsa_ctm_send_row_update_common (conf, pdsa_fill_amb_evpn_mj); 

    NBB_TRC_EXIT();
    return;
}

NBB_VOID
pdsa_evpn_create (pdsa_config_t *conf)
{
    NBB_TRC_ENTRY ("pdsa_evpn_create");

    // evpnEntTable
    conf->entity_index  = PDSA_EVPN_ENT_INDEX;
    pdsa_row_update_evpn (conf);

    // evpnMjTable - AMB_EVPN_IF_ATG_BDPI
    conf->interface_id   = AMB_EVPN_IF_ATG_BDPI;
    conf->partner_type   = AMB_EVPN_MJ_PARTNER_L2FST;
    conf->partner_index  = 1;
    conf->sub_index      = 0;
    pdsa_row_update_evpn_mj (conf);

    // evpnMjTable - AMB_EVPN_IF_ATG_I3
    conf->interface_id   = AMB_EVPN_IF_ATG_I3;
    conf->partner_type   = AMB_EVPN_MJ_PARTNER_LIM;
    conf->partner_index  = 1;
    conf->sub_index      = 0;
    pdsa_row_update_evpn_mj (conf);

    // evpnMjTable -AMB_EVPN_IF_ATG_TPI 
    conf->interface_id   = AMB_EVPN_IF_ATG_TPI;
    conf->partner_type   = AMB_EVPN_MJ_PARTNER_LIM;
    conf->partner_index  = 1;
    conf->sub_index      = 0;
    pdsa_row_update_evpn_mj (conf);

    // evpnMjTable - AMB_EVPN_IF_ATG_MAI
    conf->interface_id   = AMB_EVPN_IF_ATG_MAI;
    conf->partner_type   = AMB_EVPN_MJ_PARTNER_L2FST;
    conf->partner_index  = 1;
    conf->sub_index      = 0;
    pdsa_row_update_evpn_mj (conf);

    NBB_TRC_EXIT();
    return;
}
}
