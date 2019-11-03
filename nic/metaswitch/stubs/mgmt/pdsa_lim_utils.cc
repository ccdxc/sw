// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// Purpose: Helper APIs for metaswitch LIM stub programming 

#include "pdsa_mgmt_utils.hpp"
#include "lim_mgmt_if.h"


// Fill limInterfaceCfgTable: AMB_LIM_IF_CFG
NBB_VOID
pdsa_fill_amb_lim_interface_cfg (AMB_GEN_IPS       *mib_msg, 
                                 AMB_LIM_IF_CFG    *v_amb_lim_if_cfg,
                                 NBB_LONG          entity_index, 
                                 NBB_LONG          if_index,
                                 NBB_LONG          row_status, 
                                 NBB_LONG          enable,
                                 NBB_LONG          ipv4_enabled,
                                 NBB_LONG          ipv4_forwarding,
                                 NBB_LONG          ipv6_enabled,
                                 NBB_LONG          ipv6_forwarding,
                                 NBB_LONG          forwarding_mode)
{ 
    NBB_TRC_ENTRY ("pdsa_fill_amb_lim_interface_cfg");

    // Set all fields absent
    AMB_SET_ALL_FIELDS_NOT_PRESENT (mib_msg);

    
    // Set all incoming fields
    v_amb_lim_if_cfg->entity_index = entity_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_LIM_IF_CFG_ENT_IX);

    v_amb_lim_if_cfg->if_index = if_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_LIM_IF_CFG_IF_IX);

    v_amb_lim_if_cfg->row_status = row_status;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_LIM_IF_CFG_RW_ST);
    
    // No need to set interface properties if it is to destroy
    if (row_status != AMB_ROW_DESTROY)
    {
        NBB_TRC_FLOW((NBB_FORMAT "Not destroying IF: fill in fields"));
        v_amb_lim_if_cfg->enable = enable;
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_LIM_IF_CFG_ENABLE);

        v_amb_lim_if_cfg->ipv4_enabled = ipv4_enabled;
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_LIM_IF_CFG_IPV4_ENBLD);

        v_amb_lim_if_cfg->ipv4_forwarding = ipv4_forwarding;
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_LIM_IF_CFG_IPV4_FWD);

        v_amb_lim_if_cfg->ipv6_enabled = ipv6_enabled;
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_LIM_IF_CFG_IPV6_ENBLD);

        v_amb_lim_if_cfg->ipv6_forwarding = ipv6_forwarding;
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_LIM_IF_CFG_IPV6_FWD);

        v_amb_lim_if_cfg->forwarding_mode = forwarding_mode;
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_LIM_IF_STUS_FWD_MODE);
    }

    NBB_TRC_EXIT ();
    return;
}

// Fill limL3InterfaceAddressTable: AMB_LIM_L3_IF_ADDR
NBB_VOID
pdsa_fill_amb_lim_l3_if_addr (AMB_GEN_IPS           *mib_msg,
                              AMB_LIM_L3_IF_ADDR    *v_amb_lim_l3_if_addr,
                              NBB_LONG              entity_index,
                              NBB_LONG              if_index,
                              ip_addr_t             ip_addr,
                              NBB_LONG              row_status,
                              NBB_LONG              prefix_len)
{ 
    NBB_TRC_ENTRY ("pdsa_fill_amb_lim_l3_if_addr");

    // Set all fields absent
    AMB_SET_ALL_FIELDS_NOT_PRESENT (mib_msg);

    // Set all incoming fields
    v_amb_lim_l3_if_addr->entity_index = entity_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_LIM_L3_ADDR_ENT_IX);

    v_amb_lim_l3_if_addr->if_index = if_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_LIM_L3_ADDR_IF_IX);

    pdsa_convert_ip_addr_to_amb_ip_addr(ip_addr, 
                                        &v_amb_lim_l3_if_addr->ipaddr_type, 
                                        &v_amb_lim_l3_if_addr->ipaddress_len,
                                        v_amb_lim_l3_if_addr->ipaddress);
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_LIM_L3_ADDR_TYPE);
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_LIM_L3_ADDR_IPADDR);

    v_amb_lim_l3_if_addr->row_status = row_status;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_LIM_L3_ADDR_ROW_ST);

    // No need to set anymore fields if it is to destroy
    if (row_status != AMB_ROW_DESTROY)
    {
        NBB_TRC_FLOW((NBB_FORMAT "Not destroying IF Address: fill in fields"));
        v_amb_lim_l3_if_addr->prefix_len = prefix_len;
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_LIM_L3_ADDR_PRFX_LEN);
    }

    NBB_TRC_EXIT ();
    return;
}

// Fill limEntTable: AMB_LIM_ENT
NBB_VOID
pdsa_fill_amb_lim_ent (AMB_GEN_IPS      *mib_msg,
                       AMB_LIM_ENT      *v_amb_lim_ent,
                       NBB_LONG         entity_index,
                       NBB_LONG         row_status)
{ 
    NBB_TRC_ENTRY ("pdsa_fill_amb_lim_ent");

    // Set all fields absent
    AMB_SET_ALL_FIELDS_NOT_PRESENT (mib_msg);

    // Set all incoming fields
    v_amb_lim_ent->entity_index = entity_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_LIM_ENT_ENTITY_INDEX);

    v_amb_lim_ent->row_status = row_status;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_LIM_ENT_ROW_STATUS);

    NBB_TRC_EXIT ();
    return;
}

// Fill limMjTable: AMB_LIM_MJ
NBB_VOID
pdsa_fill_amb_lim_mj (AMB_GEN_IPS   *mib_msg,
                      AMB_LIM_MJ    *v_amb_lim_mj,
                      NBB_ULONG     lim_entity_index,
                      NBB_LONG      interface_id,
                      NBB_ULONG     partner_type,
                      NBB_ULONG     partner_index,
                      NBB_ULONG     sub_index,
                      NBB_LONG      row_status)
{ 
    NBB_TRC_ENTRY ("pdsa_fill_amb_lim_mj");

    // Set all fields absent
    AMB_SET_ALL_FIELDS_NOT_PRESENT (mib_msg);

    // Set all incoming fields
    v_amb_lim_mj->lim_entity_index = lim_entity_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_LIM_MJ_ENTITY_INDEX);

    v_amb_lim_mj->interface_id = interface_id;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_LIM_MJ_INTERFACE);

    v_amb_lim_mj->partner_type = partner_type;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_LIM_MJ_PARTNER_TYPE);

    v_amb_lim_mj->partner_index = partner_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_LIM_MJ_PARTNER_INDEX);

    v_amb_lim_mj->sub_index = sub_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_LIM_MJ_SUB_INDEX);

    v_amb_lim_mj->row_status = row_status;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_LIM_MJ_ROW_STATUS);

    NBB_TRC_EXIT ();
    return;
}


// Fill limIrbInterfaceTable: AMB_LIM_IRB_IF
NBB_VOID 
pdsa_fill_amb_lim_irb_if (AMB_GEN_IPS       *mib_msg,
                          AMB_LIM_IRB_IF    *v_amb_lim_irb_if,
                          NBB_LONG          entity_index,
                          NBB_ULONG         vlan,
                          NBB_LONG          row_status)
{ 
    NBB_TRC_ENTRY ("pdsa_fill_amb_lim_irb_if");

    // Set all fields absent
    AMB_SET_ALL_FIELDS_NOT_PRESENT (mib_msg);

    // Set all incoming fields
    v_amb_lim_irb_if->entity_index = entity_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_LIM_IRB_IF_ENT_IX);

    v_amb_lim_irb_if->vlan = vlan;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_LIM_IRB_IF_VLAN);

    v_amb_lim_irb_if->row_status = row_status;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_LIM_IRB_IF_RW_ST);

    NBB_TRC_EXIT ();
    return;
}


// Fill LimSoftwareInterfaceTable: AMB_LIM_SOFTWARE_IF
NBB_VOID
pdsa_fill_amb_lim_sw_if (AMB_GEN_IPS            *mib_msg,
                         AMB_LIM_SOFTWARE_IF    *v_amb_lim_sw_if,
                         NBB_LONG               entity_index,
                         NBB_LONG               type,
                         NBB_LONG               index,
                         NBB_LONG               row_status)
{ 
    NBB_TRC_ENTRY ("pdsa_fill_amb_lim_sw_if");

    // Set all fields absent
    AMB_SET_ALL_FIELDS_NOT_PRESENT (mib_msg);

    // Set all incoming fields
    v_amb_lim_sw_if->entity_index = entity_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_LIM_SOFTWIF_ENT_IX);

    v_amb_lim_sw_if->type = type;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_LIM_SOFTWIF_TYPE);

    v_amb_lim_sw_if->index = index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_LIM_SOFTWIF_IX);

    v_amb_lim_sw_if->row_status = row_status;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_LIM_SOFTWIF_RW_ST);

    NBB_TRC_EXIT ();
    return;
}
