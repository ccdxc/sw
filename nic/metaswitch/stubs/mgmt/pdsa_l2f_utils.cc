// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// Purpose: Helper APIs for metaswitch L2F stub programming 

#include "nic/metaswitch/stubs/mgmt/pdsa_mgmt_utils.hpp"
#include "l2f_mgmt_if.h"
#include <iostream>

using namespace std;

// Fill l2fEntTable: AMB_STUBS_L2F_ENT
NBB_VOID
pdsa_fill_amb_l2f_ent (AMB_GEN_IPS *mib_msg, pdsa_config_t *conf)
{ 
    // Local variables
    NBB_ULONG           *oid = NULL; 
    AMB_STUBS_L2F_ENT   *data = NULL;

    // Get oid and data offset 
    oid     = (NBB_ULONG *)((NBB_BYTE *)mib_msg + mib_msg->oid_offset);
    data    = (AMB_STUBS_L2F_ENT *)((NBB_BYTE *)mib_msg + mib_msg->data_offset); 

    NBB_TRC_ENTRY ("pdsa_fill_amb_l2f_ent");

    // Set all fields absent
    AMB_SET_ALL_FIELDS_NOT_PRESENT (mib_msg);

    // Set OID len and family
    oid[0] = AMB_L2F_ENT_OID_LEN;
    oid[1] = AMB_FAM_STUBS_L2F_ENT;

    // Set all incoming fields
    data->entity_index              = conf->entity_index;
    oid[AMB_L2F_ENT_ENT_IX_INDEX]   = conf->entity_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_L2F_ENT_ENTITY_INDEX);

    data->row_status = conf->row_status;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_L2F_ENT_ROW_STATUS);

    if (conf->row_status != AMB_ROW_DESTROY)
    {
        NBB_TRC_FLOW ((NBB_FORMAT "Not destroying L2F Ent: fill in fields"));
        data->stateful = conf->stateful;
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_L2F_ENT_STATEFUL);
    }

    NBB_TRC_EXIT ();
    return;
}

// Fill l2fMacIpCfgTable: AMB_STUBS_L2F_MAC_IP_CFG
NBB_VOID
pdsa_fill_amb_l2f_mac_ip_cfg (AMB_GEN_IPS *mib_msg, pdsa_config_t *conf)
{
    // Local variables
    NBB_ULONG                   *oid = NULL; 
    AMB_STUBS_L2F_MAC_IP_CFG    *data = NULL;
    NBB_ULONG                   ii = 0;

    NBB_TRC_ENTRY ("pdsa_fill_amb_l2f_mac_ip_cfg");

    // Get oid and data offset 
    oid     = (NBB_ULONG *)((NBB_BYTE *)mib_msg + mib_msg->oid_offset);
    data    = (AMB_STUBS_L2F_MAC_IP_CFG*)((NBB_BYTE *)mib_msg + mib_msg->data_offset); 

    // Set all fields absent
    AMB_SET_ALL_FIELDS_NOT_PRESENT (mib_msg);

    // Set OID len and family
    oid[0] = AMB_L2F_MAC_IP_CFG_OID_LEN;
    oid[1] = AMB_FAM_STUBS_L2F_MAC_IP_CFG;

    // Set all incoming fields
    data->ent_index                         = conf->entity_index;
    oid[AMB_L2F_MAC_IP_CFG_ENT_IX_INDEX]    = conf->entity_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_L2F_MAC_IP_CFG_ENT_IX);

    data->bd_type                           = conf->bd_type;
    oid[AMB_L2F_MAC_IP_CFG_BD_TYP_INDEX]    = conf->bd_type;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_L2F_MAC_IP_CFG_BD_TYP);

    data->bd_index                      = conf->bd_index;
    oid[AMB_L2F_MAC_IP_CFG_BD_IX_INDEX] = conf->bd_index; 
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_L2F_MAC_IP_CFG_BD_IX);

    data->bd_sub_index                  = conf->bd_sub_index;
    oid[AMB_L2F_MAC_IP_CFG_BD_SB_INDEX] = conf->bd_sub_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_L2F_MAC_IP_CFG_BD_SUB);

    for (ii = 0; ii < AMB_MAC_ADDR_LEN; ii++)
    {
        data->mac_address[ii]                       = conf->mac_address[ii];
        oid[AMB_L2F_MAC_IP_CFG_MC_ADD_INDEX + ii]   = (NBB_ULONG)conf->mac_address[ii];
    }
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_L2F_MAC_IP_CFG_MAC_ADD);

    pdsa_convert_ip_addr_to_amb_ip_addr(conf->ip_addr, 
                                        &data->ip_address_type, 
                                        &data->ip_address_len,
                                        data->ip_address);
    oid[AMB_L2F_MAC_IP_CFG_IP_TYP_INDEX] = data->ip_address_type;
    oid[AMB_L2F_MAC_IP_CFG_IP_ADD_INDEX] = data->ip_address_len;
    for (ii = 0; ii < data->ip_address_len; ii++)
    {

        oid [AMB_L2F_MAC_IP_CFG_IP_ADD_INDEX + 1 + ii] = 
           (NBB_ULONG) data->ip_address[ii];
    }
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_L2F_MAC_IP_CFG_IP_TYPE);
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_L2F_MAC_IP_CFG_IP_ADDR);

    data->row_status = conf->row_status;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_L2F_MAC_IP_CFG_RW_STA);

    if (conf->row_status != AMB_ROW_DESTROY)
    {
        NBB_TRC_FLOW((NBB_FORMAT "Not destroying MAI: fill in fields"));
        data->if_index = conf->if_index;
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_L2F_MAC_IP_CFG_IF_IX);
    }

    NBB_TRC_EXIT ();
    return;
}

NBB_VOID
pdsa_test_row_update_l2f (pdsa_config_t *conf)
{
    NBB_TRC_ENTRY ("pdsa_test_row_update_l2f");

    conf->oid_len       = AMB_L2F_ENT_OID_LEN;
    conf->data_len      = sizeof (AMB_STUBS_L2F_ENT);
    conf->entity_index  = 1;
    conf->row_status    = AMB_ROW_ACTIVE;
    conf->stateful      = AMB_FALSE;

    pdsa_ctm_send_row_update_common (conf, pdsa_fill_amb_l2f_ent); 

    NBB_TRC_EXIT();
    return;
}


NBB_VOID
pdsa_test_row_update_l2f_mac_ip_cfg (pdsa_config_t *conf)
{
    // Local variables
    NBB_BYTE                    mac_addr[AMB_MAC_ADDR_LEN] = {0x12, 0x34, 0x56, 0x78, 0x90, 0x12};

    NBB_TRC_ENTRY ("pdsa_test_row_update_l2f_mac_ip_cfg");

    // Set params
    conf->oid_len       = AMB_L2F_MAC_IP_CFG_OID_LEN;
    conf->data_len      = sizeof (AMB_STUBS_L2F_MAC_IP_CFG);
    conf->entity_index  = 1;
    conf->row_status    = AMB_ROW_ACTIVE;
    conf->bd_type       = AMB_L2_BRIDGE_DOMAIN_EVPN;
    conf->bd_index      = 1;
    conf->bd_sub_index  = 0;
    conf->if_index      = 7;

    pdsa_convert_long_to_pdsa_ipv4_addr (conf->g_node_a_ac_ip, &conf->ip_addr);
    NBB_MEMCPY (conf->mac_address, mac_addr, AMB_MAC_ADDR_LEN);

    // Convert to row_update and send
    pdsa_ctm_send_row_update_common (conf, pdsa_fill_amb_l2f_mac_ip_cfg); 

    NBB_TRC_EXIT();
    return;
}
