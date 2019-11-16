// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//Purpose: Helper APIs for metaswitch EVPN component

#include "nic/metaswitch/stubs/mgmt/pdsa_mgmt_utils.hpp"
#include "evpn_mgmt_if.h"

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

    pdsa_convert_ip_addr_to_amb_ip_addr (conf->ip_addr,
                                         &data->local_router_address_type,
                                         &data->local_router_addr_len,
                                         data->local_router_addr);
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_EVPN_ENT_LCL_RTR_ADR_TY);
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_EVPN_ENT_LCL_RTR_ADDR);

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


// Fill evpnEviTable: AMB_EVPN_EVI
NBB_VOID
pdsa_fill_amb_evpn_evi (AMB_GEN_IPS *mib_msg, pdsa_config_t *conf)
{ 
    // Local variables
    NBB_ULONG       *oid = NULL; 
    AMB_EVPN_EVI    *data= NULL;

    NBB_TRC_ENTRY ("pdsa_fill_amb_evpn_evi");

    // Get oid and data offset 
    oid     = (NBB_ULONG *)((NBB_BYTE *)mib_msg + mib_msg->oid_offset);
    data    = (AMB_EVPN_EVI *)((NBB_BYTE *)mib_msg + mib_msg->data_offset); 

    // Set all fields absent
    AMB_SET_ALL_FIELDS_NOT_PRESENT (mib_msg);

    // Set OID len and family
    oid[0] = AMB_EVPN_EVI_OID_LEN;
    oid[1] = AMB_FAM_EVPN_EVI;

    // Set all incoming fields
    oid[AMB_EVPN_EVI_ENTITY_IX_INDEX]   = conf->entity_index;
    data->entity_index                  = conf->entity_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_EVPN_EVI_ENTITY_IX);

    oid[AMB_EVPN_EVI_INDEX_INDEX]   = conf->evi_index;
    data->index                     = conf->evi_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_EVPN_EVI_INDEX);

    data->row_status = conf->row_status;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_EVPN_EVI_ROW_STATUS);

    if (data->row_status != AMB_ROW_DESTROY)
    {

        NBB_TRC_FLOW ((NBB_FORMAT "Not destroying EVPN EVI: fill in fields"));
        data->rd_cfg_or_auto = conf->rd_cfg_or_auto;
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_EVPN_EVI_RD_CFG_AUTO);

        if (data->rd_cfg_or_auto == AMB_EVPN_CONFIGURED)
        {
            NBB_MEMCPY (data->cfg_rd, conf->cfg_rd,  
                        AMB_EVPN_EXT_COMM_LENGTH);
            AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_EVPN_EVI_CFG_RD);
        }

        data->encapsulation = conf->encapsulation;
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_EVPN_EVI_ENCAPS);
    }

    NBB_TRC_EXIT ();
    return;
}

// Fill evpnBdTable: AMB_EVPN_BD
NBB_VOID
pdsa_fill_amb_evpn_bd (AMB_GEN_IPS  *mib_msg, pdsa_config_t *conf)
{ 
    // Local variables
    NBB_ULONG   *oid = NULL; 
    AMB_EVPN_BD *data= NULL;

    NBB_TRC_ENTRY ("pdsa_fill_amb_evpn_bd");

    // Get oid and data offset 
    oid     = (NBB_ULONG *)((NBB_BYTE *)mib_msg + mib_msg->oid_offset);
    data    = (AMB_EVPN_BD *)((NBB_BYTE *)mib_msg + mib_msg->data_offset); 

    // Set all fields absent
    AMB_SET_ALL_FIELDS_NOT_PRESENT (mib_msg);

    // Set OID len and family
    oid[0] = AMB_EVPN_BD_OID_LEN;
    oid[1] = AMB_FAM_EVPN_BD;

    // Set all incoming fields
    oid[AMB_EVPN_BD_ENTITY_IX_INDEX]    = conf->entity_index;
    data->entity_index                  = conf->entity_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_EVPN_BD_ENT_ENTITY_IX);

    oid[AMB_EVPN_BD_EVI_INDEX_INDEX]    = conf->evi_index;
    data->evi_index                     = conf->evi_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_EVPN_BD_EVI_INDEX);

    data->row_status = conf->row_status;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_EVPN_BD_ROW_STATUS);

    if (data->row_status != AMB_ROW_DESTROY)
    {
        NBB_TRC_FLOW ((NBB_FORMAT "Not destroying EVPN BD: fill in fields"));
        data->vni = conf->vni;
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_EVPN_BD_VNI);
    }

    NBB_TRC_EXIT ();
    return;
}


// Fill evpnIfBindCfgTable: AMB_EVPN_IF_BIND_CFG
NBB_VOID
pdsa_fill_amb_evpn_if_bind_cfg (AMB_GEN_IPS *mib_msg, pdsa_config_t *conf)
{ 
    // Local variables
    NBB_ULONG               *oid = NULL; 
    AMB_EVPN_IF_BIND_CFG    *data= NULL;

    NBB_TRC_ENTRY ("pdsa_fill_amb_evpn_if_bind_cfg");

    // Get oid and data offset 
    oid     = (NBB_ULONG *)((NBB_BYTE *)mib_msg + mib_msg->oid_offset);
    data    = (AMB_EVPN_IF_BIND_CFG *)((NBB_BYTE *)mib_msg + mib_msg->data_offset); 

    // Set all fields absent
    AMB_SET_ALL_FIELDS_NOT_PRESENT (mib_msg);

    // Set OID len and family
    oid[0] = AMB_EVPN_IF_BIND_CFG_OID_LEN;
    oid[1] = AMB_FAM_EVPN_IF_BIND_CFG;

    // Set all incoming fields
    oid[AMB_EVPN_IF_BIND_CFG_ENT_INDEX] = conf->entity_index;
    data->entity_index                  = conf->entity_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_EVPN_IBC_ENT_IX);

    oid[AMB_EVPN_IF_BIND_CFG_IF_INDEX]  = conf->if_index;
    data->if_index                      = conf->if_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_EVPN_IBC_IF_INDEX);

    data->row_status = conf->row_status;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_EVPN_IBC_ROW_STATUS);

    if (data->row_status != AMB_ROW_DESTROY)
    {
        NBB_TRC_FLOW ((NBB_FORMAT "Not destroying EVPN IF BIND CFG: fill in fields"));
        data->evi_index = conf->evi_index;
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_EVPN_IBC_EVI_IX);
    }
    NBB_TRC_EXIT ();
    return;
}

// Fill evpnIpVrfTable: AMB_EVPN_IP_VRF
NBB_VOID
pdsa_fill_amb_evpn_ip_vrf (AMB_GEN_IPS *mib_msg, pdsa_config_t *conf)
{ 
    // Local variables
    NBB_ULONG       *oid = NULL; 
    AMB_EVPN_IP_VRF *data= NULL;
    NBB_ULONG       ii = 0;

    NBB_TRC_ENTRY ("pdsa_fill_amb_evpn_ip_vrf");

    // Get oid and data offset 
    oid     = (NBB_ULONG *)((NBB_BYTE *)mib_msg + mib_msg->oid_offset);
    data    = (AMB_EVPN_IP_VRF *)((NBB_BYTE *)mib_msg + mib_msg->data_offset); 

    // Set all fields absent
    AMB_SET_ALL_FIELDS_NOT_PRESENT (mib_msg);

    // Set OID len and family
    oid[0] = AMB_EVPN_IP_VRF_OID_LEN;
    oid[1] = AMB_FAM_EVPN_IP_VRF;

    // Set all incoming fields
    oid[AMB_EVPN_IP_VRF_ENT_INDEX]  = conf->entity_index;
    data->entity_index              = conf->entity_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_EVPN_IP_VRF_ENT_IX);

    oid[AMB_EVPN_IP_VRF_NAME_LEN_INDEX]     = conf->vrf_name_len;
    data->vrf_name_len                      = conf->vrf_name_len;

    for (ii = 0; ii < conf->vrf_name_len; ii++)
    {
        oid[AMB_EVPN_IP_VRF_NAME_INDEX + ii] = (NBB_ULONG)conf->vrf_name[ii];
    }
    NBB_MEMCPY (data->vrf_name, conf->vrf_name, conf->vrf_name_len);
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_EVPN_IP_VRF_NAME);

    data->row_status = conf->row_status;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_EVPN_IP_VRF_ROW_STATUS);

    if (data->row_status != AMB_ROW_DESTROY)
    {
        NBB_TRC_FLOW ((NBB_FORMAT "Not destroying IP VRF Entry: fill in fields"));
        data->vni = conf->vni;
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_EVPN_IP_VRF_VNI);

        NBB_MEMCPY (data->route_distinguisher, conf->route_distinguisher, 
                    AMB_EVPN_EXT_COMM_LENGTH);
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_EVPN_IP_VRF_RD);
    }

    NBB_TRC_EXIT ();
    return;
}

// TODO: Do we need AMB_EVPN_MAC_IP (evpnMacIpTable)?


// row_update for evpnEntTable
NBB_VOID
pdsa_test_row_update_evpn (pdsa_config_t *conf)
{
    NBB_TRC_ENTRY ("pdsa_test_row_update_evpn");

    // Set params
    conf->oid_len       = AMB_EVPN_ENT_OID_LEN;
    conf->data_len      = sizeof (AMB_EVPN_ENT);
    conf->entity_index  = 1;
    conf->row_status    = AMB_ROW_ACTIVE;

    pdsa_convert_long_to_pdsa_ipv4_addr (NODE_A_IP, &conf->ip_addr);

    // Convert to row_update and send
    pdsa_ctm_send_row_update_common (conf, pdsa_fill_amb_evpn_ent); 

    NBB_TRC_EXIT();
    return;
}

NBB_VOID
pdsa_test_row_update_evpn_mj (pdsa_config_t *conf,
                              NBB_ULONG     interface_id,
                              NBB_ULONG     partner_type,
                              NBB_ULONG     partner_index,
                              NBB_ULONG     sub_index)
{
    NBB_TRC_ENTRY ("pdsa_test_row_update_evpn_mj");

    // Set params
    conf->oid_len       = AMB_EVPN_MJ_OID_LEN;
    conf->data_len      = sizeof (AMB_EVPN_MJ);
    conf->entity_index  = 1;
    conf->row_status    = AMB_ROW_ACTIVE;
    conf->interface_id  = interface_id;
    conf->partner_type  = partner_type;
    conf->partner_index = partner_index;
    conf->sub_index     = sub_index;

    // Convert to row_update and send
    pdsa_ctm_send_row_update_common (conf, pdsa_fill_amb_evpn_mj); 

    NBB_TRC_EXIT();
    return;
}

// row_update for evpnEviTable
NBB_VOID
pdsa_test_row_update_evpn_evi (pdsa_config_t *conf)
{
    NBB_TRC_ENTRY ("pdsa_test_row_update_evpn_evi");

    // Set params
    conf->oid_len           = AMB_EVPN_EVI_OID_LEN;;
    conf->data_len          = sizeof (AMB_EVPN_EVI);
    conf->entity_index      = 1;
    conf->row_status        = AMB_ROW_ACTIVE;
    conf->evi_index         = 1;
    conf->rd_cfg_or_auto    = AMB_EVPN_AUTO; 
    conf->encapsulation     = AMB_EVPN_ENCAPS_VXLAN;


    // Convert to row_update and send
    pdsa_ctm_send_row_update_common (conf, pdsa_fill_amb_evpn_evi); 

    NBB_TRC_EXIT();
    return;
}


// row_update for evpnBdTable
NBB_VOID
pdsa_test_row_update_evpn_bd (pdsa_config_t *conf)
{
    NBB_TRC_ENTRY ("pdsa_test_row_update_evpn_bd");

    // Set params
    conf->oid_len       = AMB_EVPN_BD_OID_LEN;
    conf->data_len      = sizeof (AMB_EVPN_BD);
    conf->entity_index  = 1;
    conf->row_status    = AMB_ROW_ACTIVE;
    conf->evi_index     = 1;
    conf->vni           = 100;

    // Convert to row_update and send
    pdsa_ctm_send_row_update_common (conf, pdsa_fill_amb_evpn_bd); 

    NBB_TRC_EXIT();
    return;
}

// row_update for evpnIfBindCfgTable
NBB_VOID
pdsa_test_row_update_evpn_if_bind_cfg (pdsa_config_t *conf)
{
    NBB_TRC_ENTRY ("pdsa_test_row_update_evpn_if_bind_cfg");

    // Set params
    conf->oid_len       = AMB_EVPN_IF_BIND_CFG_OID_LEN;
    conf->data_len      = sizeof (AMB_EVPN_IF_BIND_CFG);
    conf->entity_index  = 1;
    conf->row_status    = AMB_ROW_ACTIVE;
    conf->if_index      = 7;
    conf->evi_index     = 1;

    // Convert to row_update and send
    pdsa_ctm_send_row_update_common (conf, pdsa_fill_amb_evpn_if_bind_cfg); 

    NBB_TRC_EXIT();
    return;
}
