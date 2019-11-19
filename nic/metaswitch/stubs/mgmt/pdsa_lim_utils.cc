// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// Purpose: Helper APIs for metaswitch LIM stub programming 

#include "nic/metaswitch/stubs/mgmt/pdsa_mgmt_utils.hpp"
#include "nic/metaswitch/stubs/pdsa_stubs_init.hpp"
#include "lim_mgmt_if.h"

// Fill limInterfaceCfgTable: AMB_LIM_IF_CFG
NBB_VOID
pdsa_fill_amb_lim_interface_cfg (AMB_GEN_IPS *mib_msg, pdsa_config_t *conf)
{ 
    // Local variables
    NBB_ULONG       *oid = NULL; 
    AMB_LIM_IF_CFG  *data= NULL;

    NBB_TRC_ENTRY ("pdsa_fill_amb_lim_interface_cfg");

    // Get oid and data offset 
    oid     = (NBB_ULONG *)((NBB_BYTE *)mib_msg + mib_msg->oid_offset);
    data    = (AMB_LIM_IF_CFG *)((NBB_BYTE *)mib_msg + mib_msg->data_offset); 

    // Set all fields absent
    AMB_SET_ALL_FIELDS_NOT_PRESENT (mib_msg);

    // Set OID len and family
    oid[0] = AMB_LIM_INTERFACE_CFG_OID_LEN;
    oid[1] = AMB_FAM_LIM_IF_CFG;
    
    // Set all incoming fields
    data->entity_index                  = conf->entity_index;
    oid[AMB_LIM_IF_CFG_ENT_IX_INDEX]    = conf->entity_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_LIM_IF_CFG_ENT_IX);

    data->if_index                  = conf->if_index;
    oid[AMB_LIM_IF_CFG_IF_IX_INDEX] = conf->if_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_LIM_IF_CFG_IF_IX);

    data->row_status = conf->row_status;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_LIM_IF_CFG_RW_ST);
    
    // No need to set interface properties if it is to destroy
    if (conf->row_status != AMB_ROW_DESTROY)
    {
        NBB_TRC_FLOW((NBB_FORMAT "Not destroying LIM IF: fill in fields"));
        data->enable = conf->enable;
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_LIM_IF_CFG_ENABLE);

        data->ipv4_enabled = conf->ipv4_enabled;
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_LIM_IF_CFG_IPV4_ENBLD);

        data->ipv4_forwarding = conf->ipv4_forwarding;
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_LIM_IF_CFG_IPV4_FWD);

        data->ipv6_enabled = conf->ipv6_enabled;
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_LIM_IF_CFG_IPV6_ENBLD);

        data->ipv6_forwarding = conf->ipv6_forwarding;
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_LIM_IF_CFG_IPV6_FWD);

        data->forwarding_mode = conf->forwarding_mode;
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_LIM_IF_CFG_FWD_MODE);
    }

    NBB_TRC_EXIT ();
    return;
}

// Fill limL3InterfaceAddressTable: AMB_LIM_L3_IF_ADDR
NBB_VOID
pdsa_fill_amb_lim_l3_if_addr (AMB_GEN_IPS *mib_msg, pdsa_config_t *conf)
{ 
    // Local variables
    NBB_ULONG           *oid = NULL; 
    AMB_LIM_L3_IF_ADDR  *data= NULL;
    NBB_ULONG           ii = 0;

    NBB_TRC_ENTRY ("pdsa_fill_amb_lim_l3_if_addr");

    // Get oid and data offset 
    oid     = (NBB_ULONG *)((NBB_BYTE *)mib_msg + mib_msg->oid_offset);
    data    = (AMB_LIM_L3_IF_ADDR *)((NBB_BYTE *)mib_msg + mib_msg->data_offset); 

    // Set all fields absent
    AMB_SET_ALL_FIELDS_NOT_PRESENT (mib_msg);

    // Set OID len and family
    oid[0] = AMB_LIM_L3_ADDRESS_OID_LEN;
    oid[1] = AMB_FAM_LIM_L3_IF_ADDR;

    // Set all incoming fields
    oid[AMB_LIM_L3_ADDR_ENT_IX_INDEX]   = conf->entity_index;
    data->entity_index                  = conf->entity_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_LIM_L3_ADDR_ENT_IX);

    oid[AMB_LIM_L3_ADDR_IF_IX_INDEX]    = conf->if_index;
    data->if_index                      = conf->if_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_LIM_L3_ADDR_IF_IX);

    pdsa_convert_ip_addr_to_amb_ip_addr(conf->ip_addr, 
                                        &data->ipaddr_type, 
                                        &data->ipaddress_len,
                                        data->ipaddress);
    oid[AMB_LIM_L3_ADDR_IPDDR_TYP_INDEX]    = data->ipaddr_type;
    oid[AMB_LIM_L3_ADDR_IPADDR_INDEX]       = data->ipaddress_len;
    for (ii = 0; ii < data->ipaddress_len; ii++)
    {
        oid[AMB_LIM_L3_ADDR_IPADDR_INDEX + 1 + ii] =
            (NBB_ULONG)data->ipaddress[ii];
    }
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_LIM_L3_ADDR_TYPE);
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_LIM_L3_ADDR_IPADDR);

    data->row_status = conf->row_status;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_LIM_L3_ADDR_ROW_ST);

    // No need to set anymore fields if it is to destroy
    if (conf->row_status != AMB_ROW_DESTROY)
    {
        NBB_TRC_FLOW((NBB_FORMAT "Not destroying IF Address: fill in fields"));
        data->prefix_len = conf->prefix_len;
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_LIM_L3_ADDR_PRFX_LEN);
    }

    NBB_TRC_EXIT ();
    return;
}

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

// Fill limIrbInterfaceTable: AMB_LIM_IRB_IF
NBB_VOID 
pdsa_fill_amb_lim_irb_if (AMB_GEN_IPS *mib_msg, pdsa_config_t *conf)
{ 
    // Local variables
    NBB_ULONG       *oid = NULL; 
    AMB_LIM_IRB_IF  *data= NULL;

    NBB_TRC_ENTRY ("pdsa_fill_amb_lim_irb_if");

    // Get oid and data offset 
    oid     = (NBB_ULONG *)((NBB_BYTE *)mib_msg + mib_msg->oid_offset);
    data    = (AMB_LIM_IRB_IF *)((NBB_BYTE *)mib_msg + mib_msg->data_offset); 

    // Set all fields absent
    AMB_SET_ALL_FIELDS_NOT_PRESENT (mib_msg);

    // Set OID len and family
    oid[0] = AMB_LIM_IRB_INTERFACE_OID_LEN;
    oid[1]= AMB_FAM_LIM_IRB_IF;

    // Set all incoming fields
    oid[AMB_LIM_IRB_IF_ENT_IX_INDEX]    = conf->entity_index;
    data->entity_index                  = conf->entity_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_LIM_IRB_IF_ENT_IX);

    oid[AMB_LIM_IRB_IF_VLAN_INDEX]  = conf->vlan;
    data->vlan                      = conf->vlan;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_LIM_IRB_IF_VLAN);

    data->row_status = conf->row_status;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_LIM_IRB_IF_RW_ST);

    NBB_TRC_EXIT ();
    return;
}


// Fill LimSoftwareInterfaceTable: AMB_LIM_SOFTWARE_IF
NBB_VOID
pdsa_fill_amb_lim_sw_if (AMB_GEN_IPS *mib_msg, pdsa_config_t *conf)
{ 
    // Local variables
    NBB_ULONG           *oid = NULL; 
    AMB_LIM_SOFTWARE_IF *data= NULL;

    NBB_TRC_ENTRY ("pdsa_fill_amb_lim_sw_if");

    // Get oid and data offset 
    oid     = (NBB_ULONG *)((NBB_BYTE *)mib_msg + mib_msg->oid_offset);
    data    = (AMB_LIM_SOFTWARE_IF *)((NBB_BYTE *)mib_msg + mib_msg->data_offset); 

    // Set all fields absent
    AMB_SET_ALL_FIELDS_NOT_PRESENT (mib_msg);

    // Set OID len and family
    oid[0] = AMB_LIM_SOFTWIF_OID_LEN;
    oid[1] = AMB_FAM_LIM_SOFTWARE_IF;

    // Set all incoming fields
    oid[AMB_LIM_SOFTWIF_ENT_IX_INDEX]   = conf->entity_index;
    data->entity_index                  = conf->entity_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_LIM_SOFTWIF_ENT_IX);

    oid[AMB_LIM_SOFTWIF_IF_TYPE]    = conf->if_type;
    data->type                      = conf->if_type;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_LIM_SOFTWIF_TYPE);

    oid[AMB_LIM_SOFTWIF_INDEX]      = conf->if_index;
    data->index                     = conf->if_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_LIM_SOFTWIF_IX);

    data->row_status = conf->row_status;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_LIM_SOFTWIF_RW_ST);

    NBB_TRC_EXIT ();
    return;
}


NBB_VOID
pdsa_test_row_update_lim (pdsa_config_t *conf)
{
    NBB_TRC_ENTRY ("pdsa_test_row_update_lim");

    // Set params
    conf->oid_len       = AMB_LIM_ENT_OID_LEN;
    conf->data_len      = sizeof (AMB_LIM_ENT);
    conf->entity_index  = 1;
    conf->row_status    = AMB_ROW_ACTIVE;

    // Convert to row_update and send
    pdsa_ctm_send_row_update_common (conf, pdsa_fill_amb_lim_ent); 

    NBB_TRC_EXIT();
    return;
}

NBB_VOID
pdsa_test_row_update_lim_mj (pdsa_config_t  *conf,
                             NBB_ULONG      interface_id,
                             NBB_ULONG      partner_type,
                             NBB_ULONG      partner_index,
                             NBB_ULONG      sub_index)
{
    NBB_TRC_ENTRY ("pdsa_test_row_update_lim_mj");

    // Set params
    conf->oid_len       = AMB_LIM_ENT_OID_LEN;
    conf->data_len      = sizeof (AMB_LIM_ENT);
    conf->entity_index  = 1;
    conf->row_status    = AMB_ROW_ACTIVE;
    conf->interface_id  = interface_id;
    conf->partner_type  = partner_type;
    conf->partner_index = partner_index;
    conf->sub_index     = sub_index;
    
    // Convert to row_update and send
    pdsa_ctm_send_row_update_common (conf, pdsa_fill_amb_lim_mj); 

    NBB_TRC_EXIT();
    return;
}

NBB_VOID
pdsa_test_row_update_lim_if_cfg (pdsa_config_t  *conf,
                                 NBB_LONG       if_index,
                                 NBB_LONG       ipv4_enabled,
                                 NBB_LONG       ipv4_forwarding,
                                 NBB_LONG       ipv6_enabled,
                                 NBB_LONG       ipv6_forwarding,
                                 NBB_LONG       forwarding_mode)
{
    NBB_TRC_ENTRY ("pdsa_test_row_update_lim_if_cfg");

    // Set params
    conf->oid_len           = AMB_LIM_INTERFACE_CFG_OID_LEN;
    conf->data_len          = sizeof (AMB_LIM_IF_CFG);
    conf->entity_index      = 1;
    conf->row_status        = AMB_ROW_ACTIVE;
    conf->if_index          = if_index;
    conf->enable            = AMB_TRUE;
    conf->ipv4_enabled      = ipv4_enabled;
    conf->ipv4_forwarding   = ipv4_forwarding;
    conf->ipv6_enabled      = ipv6_enabled;
    conf->ipv6_forwarding   = ipv6_forwarding;
    conf->forwarding_mode   = forwarding_mode;

    // Convert to row_update and send
    pdsa_ctm_send_row_update_common (conf, pdsa_fill_amb_lim_interface_cfg); 

    NBB_TRC_EXIT();
    return;
}

NBB_VOID
pdsa_test_row_update_lim_if_addr (pdsa_config_t *conf)
{
    NBB_TRC_ENTRY ("pdsa_test_row_update_lim_if_addr");

    // Set params
    conf->oid_len       = AMB_LIM_L3_ADDRESS_OID_LEN;
    conf->data_len      = sizeof (AMB_LIM_L3_IF_ADDR);
    conf->entity_index  = 1;
    conf->row_status    = AMB_ROW_ACTIVE;
    conf->if_index      = conf->g_evpn_if_index;
    conf->prefix_len    = 24;
    
    pdsa_convert_long_to_pdsa_ipv4_addr (conf->g_node_a_ip, &conf->ip_addr);

    // Convert to row_update and send
    pdsa_ctm_send_row_update_common (conf, pdsa_fill_amb_lim_l3_if_addr); 

    NBB_TRC_EXIT();
    return;
}
