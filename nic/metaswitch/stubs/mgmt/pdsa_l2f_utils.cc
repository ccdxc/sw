// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// Purpose: Helper APIs for metaswitch L2F stub programming 

#include "pdsa_mgmt_utils.hpp"
#include "l2f_mgmt_if.h"

// Fill l2fEntTable: AMB_STUBS_L2F_ENT
NBB_VOID
pdsa_fill_amb_l2f_ent (AMB_GEN_IPS          *mib_msg,
                       AMB_STUBS_L2F_ENT    *v_amb_l2f_ent,
                       NBB_ULONG            entity_index,
                       NBB_LONG             row_status,
                       NBB_LONG             stateful)
{ 
    NBB_TRC_ENTRY ("pdsa_fill_amb_l2f_ent");

    // Set all fields absent
    AMB_SET_ALL_FIELDS_NOT_PRESENT (mib_msg);

    // Set all incoming fields
    v_amb_l2f_ent->entity_index = entity_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_L2F_ENT_ENTITY_INDEX);

    v_amb_l2f_ent->row_status = row_status;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_L2F_ENT_ROW_STATUS);

    if (row_status != AMB_ROW_DESTROY)
    {
        NBB_TRC_FLOW ((NBB_FORMAT "Not destroying L2F Ent: fill in fields"));
        v_amb_l2f_ent->stateful = stateful;
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_L2F_ENT_STATEFUL);
    }

    NBB_TRC_EXIT ();
    return;
}

// Fill l2fMacIpCfgTable: AMB_STUBS_L2F_MAC_IP_CFG
NBB_VOID
pdsa_fill_amb_l2f_mac_ip_cfg (AMB_GEN_IPS               *mib_msg,
                              AMB_STUBS_L2F_MAC_IP_CFG  *v_amb_l2f_mac_ip_cfg,
                              NBB_LONG                  ent_index,
                              NBB_ULONG                 bd_type,
                              NBB_ULONG                 bd_index,
                              NBB_ULONG                 bd_sub_index,
                              NBB_BYTE                  *mac_address,
                              ip_addr_t                 ip_addr,
                              NBB_LONG                  row_status,
                              NBB_LONG                  if_index)
{
    NBB_TRC_ENTRY ("pdsa_fill_amb_l2f_mac_ip_cfg");

    // Set all fields absent
    AMB_SET_ALL_FIELDS_NOT_PRESENT (mib_msg);

    // Set all incoming fields
    v_amb_l2f_mac_ip_cfg->ent_index = ent_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_L2F_MAC_IP_CFG_ENT_IX);

    v_amb_l2f_mac_ip_cfg->bd_type = bd_type;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_L2F_MAC_IP_CFG_BD_TYP);

    v_amb_l2f_mac_ip_cfg->bd_index = bd_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_L2F_MAC_IP_CFG_BD_IX);

    v_amb_l2f_mac_ip_cfg->bd_sub_index = bd_sub_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_L2F_MAC_IP_CFG_BD_SUB);

    NBB_MEMCPY (v_amb_l2f_mac_ip_cfg->mac_address, mac_address, AMB_MAC_ADDR_LEN);
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_L2F_MAC_IP_CFG_MAC_ADD);

    pdsa_convert_ip_addr_to_amb_ip_addr(ip_addr, 
                                        &v_amb_l2f_mac_ip_cfg->ip_address_type, 
                                        &v_amb_l2f_mac_ip_cfg->ip_address_len,
                                        v_amb_l2f_mac_ip_cfg->ip_address);
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_L2F_MAC_IP_CFG_IP_TYPE);
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_L2F_MAC_IP_CFG_IP_ADDR);

    v_amb_l2f_mac_ip_cfg->row_status = row_status;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_L2F_MAC_IP_CFG_RW_STA);

    if (row_status != AMB_ROW_DESTROY)
    {
        NBB_TRC_FLOW((NBB_FORMAT "Not destroying MAI: fill in fields"));
        v_amb_l2f_mac_ip_cfg->if_index = if_index;
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_L2F_MAC_IP_CFG_IF_IX);
    }

    NBB_TRC_EXIT ();
    return;
}
