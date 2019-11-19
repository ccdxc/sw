// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//Purpose: Helper APIs for metaswitch BGP-RM/NM components

#include "nic/metaswitch/stubs/mgmt/pdsa_mgmt_utils.hpp"
#include "nic/metaswitch/stubs/pdsa_stubs_init.hpp"
#include "qb0mib.h"

// Fill bgpRmEntTable: AMB_BGP_RM_ENT
NBB_VOID
pdsa_fill_amb_bgp_rm (AMB_GEN_IPS *mib_msg, pdsa_config_t *conf)
{
    // Local variables
    NBB_ULONG       *oid = NULL; 
    AMB_BGP_RM_ENT  *data= NULL;

    NBB_TRC_ENTRY ("pdsa_fill_amb_bgp_rm");

    // Get oid and data offset 
    oid     = (NBB_ULONG *)((NBB_BYTE *)mib_msg + mib_msg->oid_offset);
    data    = (AMB_BGP_RM_ENT *)((NBB_BYTE *)mib_msg + mib_msg->data_offset); 

    // Set all fields absent
    AMB_SET_ALL_FIELDS_NOT_PRESENT (mib_msg);

    // Set OID len and family
    oid[0] = AMB_BGP_RM_OID_LEN;
    oid[1] = AMB_FAM_BGP_RM_ENT;

    // Set all incoming fields
    oid[AMB_BGP_RM_INDEX_INDEX] = conf->entity_index;
    data->index                 = conf->entity_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_RM_INDEX);

    data->row_status = conf->row_status;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_RM_ROW_STATUS);

    if (conf->row_status != AMB_ROW_DESTROY)
    {
        NBB_TRC_FLOW ((NBB_FORMAT "Not destroying RM: fill in fields"));
        data->admin_status = conf->admin_status;
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_RM_ADMIN_STATUS);

        data->local_as = conf->local_as;
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_RM_LOCAL_AS);

        NBB_PUT_LONG(data->local_identifier, conf->router_id);
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_RM_LOCAL_IDENTIFIR);

        data->i3_ent_index = conf->i3_index;
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_RM_I3_ENT_INDEX);

        data->restart_supported = conf->restart_supported;
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_RM_RESTART_SUPP);
    }

    NBB_TRC_EXIT();
    return;
} 

// Fill  bgpNmEntTable: AMB_BGP_NM_ENT
NBB_VOID
pdsa_fill_amb_bgp_nm (AMB_GEN_IPS *mib_msg, pdsa_config_t *conf)
{
    // Local variables
    NBB_ULONG       *oid = NULL; 
    AMB_BGP_NM_ENT  *data= NULL;

    NBB_TRC_ENTRY ("pdsa_fill_amb_bgp_nm");

    // Get oid and data offset 
    oid     = (NBB_ULONG *)((NBB_BYTE *)mib_msg + mib_msg->oid_offset);
    data    = (AMB_BGP_NM_ENT *)((NBB_BYTE *)mib_msg + mib_msg->data_offset); 

    // Set all fields absent
    AMB_SET_ALL_FIELDS_NOT_PRESENT (mib_msg);

    // Set OID len and family
    oid[0] = AMB_BGP_NM_OID_LEN;
    oid[1] = AMB_FAM_BGP_NM_ENT;

    // Set all incoming fields
    oid[AMB_BGP_NM_INDEX_INDEX] = conf->entity_index;
    data->index                 = conf->entity_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_NM_INDEX);

    data->row_status = conf->row_status;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_NM_ROW_STATUS);

    if (data->row_status != AMB_ROW_DESTROY)
    {
        NBB_TRC_FLOW ((NBB_FORMAT "Not destroying NM: fill in fields"));
        data->rm_index = conf->rm_index;
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_NM_RM_INDEX);

        data->admin_status = conf->admin_status;
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_NM_ADMIN_STATUS);

        data->sck_index = conf->sck_index;
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_NM_SCK_INDEX);
    }

    NBB_TRC_EXIT();
    return;
}


// Fill bgpRmAfiSafiTable: AMB_BGP_RM_AFI_SAFI
NBB_VOID
pdsa_fill_amb_bgp_rm_afi_safi (AMB_GEN_IPS *mib_msg, pdsa_config_t *conf)
{
    // Local variables
    NBB_ULONG           *oid = NULL; 
    AMB_BGP_RM_AFI_SAFI *data= NULL;

    NBB_TRC_ENTRY ("pdsa_fill_amb_bgp_rm_afi_safi");

    // Get oid and data offset 
    oid     = (NBB_ULONG *)((NBB_BYTE *)mib_msg + mib_msg->oid_offset);
    data    = (AMB_BGP_RM_AFI_SAFI *)((NBB_BYTE *)mib_msg + mib_msg->data_offset); 

    // Set all fields absent
    AMB_SET_ALL_FIELDS_NOT_PRESENT (mib_msg);

    // Set OID len and family
    oid[0] = AMB_BGP_AFI_OID_LEN;
    oid[1] = AMB_FAM_BGP_RM_AFI_SAFI;

    // Set all incoming fields
    oid[AMB_BGP_AFI_ENT_INDEX_INDEX]    = conf->entity_index;
    data->ent_index                     = conf->entity_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_AFI_ENT_INDEX);

    oid[AMB_BGP_AFI_AFI_INDEX]  = conf->afi;
    data->afi                   = conf->afi;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_AFI_AFI);

    oid[AMB_BGP_AFI_SAFI_INDEX] = conf->safi;
    data->safi                  = conf->safi;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_AFI_SAFI);

    data->next_hop_safi = conf->next_hop_safi;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_AFI_NEXT_HOP_SAFI);

    data->admin_status = conf->admin_status;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_AFI_ADMIN_STATUS);

    data->state_kept = conf->state_kept;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_AFI_STATE_KEPT);

    NBB_TRC_EXIT();
    return;
}


// Fill bgpNmListenTable: AMB_BGP_NM_LISTEN
NBB_VOID
pdsa_fill_amb_bgp_nm_listen (AMB_GEN_IPS *mib_msg, pdsa_config_t *conf)
{
    // Local variables
    NBB_ULONG           *oid = NULL; 
    AMB_BGP_NM_LISTEN   *data= NULL;

    NBB_TRC_ENTRY ("pdsa_fill_amb_bgp_nm_listen");

    // Get oid and data offset 
    oid     = (NBB_ULONG *)((NBB_BYTE *)mib_msg + mib_msg->oid_offset);
    data    = (AMB_BGP_NM_LISTEN *)((NBB_BYTE *)mib_msg + mib_msg->data_offset); 

    // Set all fields absent
    AMB_SET_ALL_FIELDS_NOT_PRESENT (mib_msg);

    // Set OID len and family
    oid[0] = AMB_BGP_LIS_OID_LEN;
    oid[1] = AMB_FAM_BGP_NM_LISTEN;

    // Set all incoming fields
    oid[AMB_BGP_LIS_ENT_INDEX_INDEX]    = conf->entity_index;
    data->ent_index                     = conf->entity_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_LIS_ENT_INDEX);

    oid[AMB_BGP_LIS_INDEX_INDEX]    = conf->sck_index;
    data->index                     = conf->sck_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_LIS_INDEX);

    data->row_status = conf->row_status;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_LIS_ROW_STATUS);

    if (data->row_status != AMB_ROW_DESTROY)
    {
        NBB_TRC_FLOW((NBB_FORMAT "Not destroying NM LISTEN: fill in fields"));
        data->admin_status = conf->admin_status;
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_LIS_ADMIN_STATUS);

        pdsa_convert_ip_addr_to_amb_ip_addr(conf->ip_addr, 
                                            &data->addr_type,
                                            &data->addr_len,
                                            data->addr);
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_LIS_ADDR_TYPE);
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_LIS_ADDR);
    }

    NBB_TRC_EXIT();
    return;
}

// Fill bgpRmAfmJoinTable: AMB_BGP_RM_AFM_JOIN
NBB_VOID
pdsa_fill_amb_bgp_rm_afm_join (AMB_GEN_IPS *mib_msg, pdsa_config_t *conf)
{
    // Local variables
    NBB_ULONG           *oid = NULL; 
    AMB_BGP_RM_AFM_JOIN *data= NULL;

    NBB_TRC_ENTRY ("pdsa_fill_amb_bgp_rm_afm_join");

    // Get oid and data offset 
    oid     = (NBB_ULONG *)((NBB_BYTE *)mib_msg + mib_msg->oid_offset);
    data    = (AMB_BGP_RM_AFM_JOIN *)((NBB_BYTE *)mib_msg + mib_msg->data_offset); 

    // Set all fields absent
    AMB_SET_ALL_FIELDS_NOT_PRESENT (mib_msg);

    // Set OID len and family
    oid[0] = AMB_BGP_AFM_OID_LEN;
    oid[1] = AMB_FAM_BGP_RM_AFM_JOIN;

    // Set all incoming fields
    oid[AMB_BGP_AFM_ENTITY_INDEX]   = conf->entity_index;
    data->ent_index                 = conf->entity_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_AFM_ENTITY);

    oid[AMB_BGP_AFM_JOIN_INDEX] = conf->join;
    data->join                  = conf->join;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_AFM_JOIN);

    data->row_status = conf->row_status;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_AFM_ROW_STATUS);

    if (data->row_status != AMB_ROW_DESTROY)
    {
        NBB_TRC_FLOW((NBB_FORMAT "Not destroying RM AFM Join: fill in fields"));
        data->admin_status = conf->admin_status;
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_AFM_ADMIN_STATUS);

        data->afi = conf->afi;
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_AFM_AFI);

        data->safi = conf->safi;
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_AFM_SAFI);

        data->partner_index = conf->partner_index;
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_AFM_PARTNER_INDEX);
    }

    NBB_TRC_EXIT();
    return;
}

// Fill bgpPeerTable: AMB_BGP_PEER
NBB_VOID
pdsa_fill_amb_bgp_peer (AMB_GEN_IPS *mib_msg, pdsa_config_t *conf)
{
    // Local variables
    NBB_ULONG       *oid = NULL; 
    AMB_BGP_PEER    *data= NULL;
    NBB_ULONG       ii = 0;

    NBB_TRC_ENTRY ("pdsa_fill_amb_bgp_peer");

    // Get oid and data offset 
    oid     = (NBB_ULONG *)((NBB_BYTE *)mib_msg + mib_msg->oid_offset);
    data    = (AMB_BGP_PEER *)((NBB_BYTE *)mib_msg + mib_msg->data_offset); 

    // Set all fields absent
    AMB_SET_ALL_FIELDS_NOT_PRESENT (mib_msg);

    // Set OID len and family
    oid[0] = AMB_BGP_PER_OID_LEN;
    oid[1] = AMB_FAM_BGP_PEER;

    // Set all incoming fields
    oid[AMB_BGP_PER_RM_ENT_INDEX_INDEX] = conf->entity_index;
    data->rm_ent_index                  = conf->entity_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_PER_RM_ENT_INDEX);

    pdsa_convert_ip_addr_to_amb_ip_addr(conf->ip_addr, 
                                        &data->local_addr_type,
                                        &data->local_addr_len,
                                        data->local_addr);
    oid[AMB_BGP_PER_LOCAL_ADD_TYP_INDEX]    = data->local_addr_type;
    oid[AMB_BGP_PER_LOCAL_ADDR_INDEX]       = data->local_addr_len;
    for (ii = 0; ii < data->local_addr_len; ii++)
    {
        oid[AMB_BGP_PER_LOCAL_ADDR_INDEX + 1 + ii] =
            (NBB_ULONG) data->local_addr[ii];
    }
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_PER_LOCAL_ADDR_TYP);
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_PER_LOCAL_ADDR);

    oid[AMB_BGP_PER_LOCAL_PORT_INDEX]   = conf->local_port;
    data->local_port                    = conf->local_port;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_PER_LOCAL_PORT);

    pdsa_convert_ip_addr_to_amb_ip_addr(conf->remote_addr, 
                                        &data->remote_addr_type,
                                        &data->remote_addr_len,
                                        data->remote_addr);
    oid[AMB_BGP_PER_REMOTE_ADD_TYP_IX]  = data->remote_addr_type;
    oid[AMB_BGP_PER_REMOTE_ADDR_INDEX]  = data->remote_addr_len;
    for (ii = 0; ii < data->remote_addr_len; ii++)
    {
        oid[AMB_BGP_PER_REMOTE_ADDR_INDEX + 1 + ii] =
            (NBB_ULONG) data->remote_addr[ii];
    }
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_PER_REMOTE_DDR_TYP);
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_PER_REMOTE_ADDR);

    oid[AMB_BGP_PER_REMOTE_PORT_INDEX]  = conf->remote_port;
    data->remote_port                   = conf->remote_port;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_PER_REMOTE_PORT);

    oid[AMB_BGP_PER_SCOPE_ID_INDEX]     = conf->local_addr_scope_id;
    data->local_addr_scope_id           = conf->local_addr_scope_id;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_PER_LOC_ADDR_SCOPE);

    data->row_status = conf->row_status;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_PER_ROW_STATUS);

    if (data->row_status != AMB_ROW_DESTROY)
    {
        NBB_TRC_FLOW((NBB_FORMAT "Not destroying BGP PEER: fill in fields"));
        data->connect_retry_interval = conf->connect_retry_interval;
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_PER_CNNCT_RTRY_NTR);

        data->remote_as = conf->remote_as;
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_PER_REMOTE_AS);

        data->config_passive = conf->config_passive;
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_PER_CFG_PASSIVE);

        data->admin_status = conf->admin_status;
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_PER_ADMIN_STATUS);

        data->local_nm = conf->local_nm;
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_PER_LOCAL_NM);
    }

    NBB_TRC_EXIT();
    return;
}

// Fill bgpPeerAfiSafiTable: AMB_BGP_PEER_AFI_SAFI
NBB_VOID
pdsa_fill_amb_bgp_peer_afi_safi (AMB_GEN_IPS *mib_msg, pdsa_config_t *conf)
{
    // Local variables
    NBB_ULONG               *oid = NULL; 
    AMB_BGP_PEER_AFI_SAFI   *data= NULL;
    NBB_ULONG               ii = 0;

    NBB_TRC_ENTRY ("pdsa_fill_amb_bgp_peer_afi_safi");

    // Get oid and data offset 
    oid     = (NBB_ULONG *)((NBB_BYTE *)mib_msg + mib_msg->oid_offset);
    data    = (AMB_BGP_PEER_AFI_SAFI *)((NBB_BYTE *)mib_msg + mib_msg->data_offset); 

    // Set all fields absent
    AMB_SET_ALL_FIELDS_NOT_PRESENT (mib_msg);

    // Set OID len and family
    oid[0] = AMB_BGP_PAS_OID_LEN;
    oid[1] = AMB_FAM_BGP_PEER_AFI_SAFI;

    // Set all incoming fields
    oid[AMB_BGP_PAS_RM_ENT_INDEX_INDEX] = conf->entity_index;
    data->rm_ent_index                  = conf->entity_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_PAS_RM_ENT_INDEX);

    pdsa_convert_ip_addr_to_amb_ip_addr(conf->ip_addr,
                                        &data->local_addr_type,
                                        &data->local_addr_len,
                                        data->local_addr);
    oid[AMB_BGP_PAS_LOCAL_ADD_TYP_INDEX]    = data->local_addr_type;                                        
    oid[AMB_BGP_PAS_LOCAL_ADDR_INDEX]       = data->local_addr_len;
    for (ii = 0; ii < data->local_addr_len; ii++)
    {
        oid[AMB_BGP_PAS_LOCAL_ADDR_INDEX + 1 + ii] = 
            (NBB_ULONG) data->local_addr[ii];
    }
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_PAS_LOCAL_ADDR_TYP);
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_PAS_LOCAL_ADDR);

    oid[AMB_BGP_PAS_LOCAL_PORT_INDEX]   = conf->local_port;
    data->local_port                    = conf->local_port;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_PER_LOCAL_PORT);

    pdsa_convert_ip_addr_to_amb_ip_addr(conf->remote_addr, 
                                        &data->remote_addr_type,
                                        &data->remote_addr_len,
                                        data->remote_addr);
    oid[AMB_BGP_PAS_REMOTE_ADD_TYP_IX]  = data->remote_addr_type;
    oid[AMB_BGP_PAS_REMOTE_ADDR_INDEX]  = data->remote_addr_len;
    for (ii = 0;  ii < data->remote_addr_len; ii++)
    {
        oid[AMB_BGP_PAS_REMOTE_ADDR_INDEX + 1 + ii] = 
            (NBB_ULONG) data->remote_addr[ii];
    }
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_PAS_REMOTE_ADDR_TYP);
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_PAS_REMOTE_ADDR);

    oid[AMB_BGP_PAS_REMOTE_PORT_INDEX]  = conf->remote_port;
    data->remote_port                   = conf->remote_port;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_PAS_REMOTE_PORT);

    oid[AMB_BGP_PAS_SCOPE_ID_INDEX] = conf->local_addr_scope_id;
    data->local_addr_scope_id       = conf->local_addr_scope_id;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_PAS_SCOPE);

    oid[AMB_BGP_PAS_AFI_INDEX]  = conf->afi;
    data->afi                   = conf->afi;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_PAS_AFI);

    oid[AMB_BGP_PAS_SAFI_INDEX] = conf->safi;
    data->safi              = conf->safi;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_PAS_SAFI);

    data->disable_afi_safi = conf->disable_afi_safi;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_PAS_DISABLE);

    //this table doesnt have row_status
    NBB_TRC_EXIT();
    return;
}


// row_update for BGP-RM
NBB_VOID
pdsa_test_row_update_bgp_rm (pdsa_config_t *conf)
{
    NBB_TRC_ENTRY ("pdsa_test_row_update_bgp_rm");

    // Set params
    conf->oid_len           = AMB_BGP_RM_OID_LEN;
    conf->data_len          = sizeof (AMB_BGP_RM_ENT);
    conf->entity_index      = 1;
    conf->row_status        = AMB_ROW_ACTIVE;
    conf->i3_index          = 1;
    conf->restart_supported = AMB_TRUE;
    conf->local_as          = 1;
    conf->admin_status      = AMB_BGP_ADMIN_STATUS_UP;
    conf->router_id         = conf->g_node_a_ip;

    // Convert to row_update and send
    pdsa_ctm_send_row_update_common (conf, pdsa_fill_amb_bgp_rm); 

    NBB_TRC_EXIT();
    return;
}

// row_update for BGP-NM
NBB_VOID
pdsa_test_row_update_bgp_nm (pdsa_config_t *conf)
{
    NBB_TRC_ENTRY ("pdsa_test_row_update_bgp_nm");

    // Set params
    conf->oid_len       = AMB_BGP_NM_OID_LEN;
    conf->data_len      = sizeof (AMB_BGP_NM_ENT);
    conf->entity_index  = 1;
    conf->row_status    = AMB_ROW_ACTIVE;
    conf->admin_status  = AMB_BGP_ADMIN_STATUS_UP;
    conf->sck_index     = 1;
    conf->rm_index      = 1;

    // Convert to row_update and send
    pdsa_ctm_send_row_update_common (conf, pdsa_fill_amb_bgp_nm); 

    NBB_TRC_EXIT();
    return;
}

// row_update for bgpRmAfiSafiTable
NBB_VOID
pdsa_test_row_update_bgp_rm_afi_safi (pdsa_config_t *conf)
{
    NBB_TRC_ENTRY ("pdsa_test_row_update_bgp_rm_afi_safi");

    // Set params
    conf->oid_len       = AMB_BGP_AFI_OID_LEN;
    conf->data_len      = sizeof (AMB_BGP_RM_AFI_SAFI);
    conf->entity_index  = 1;
    conf->afi           = AMB_BGP_AFI_L2VPN;
    conf->safi          = AMB_BGP_EVPN;
    conf->next_hop_safi = AMB_BGP_UNICAST;
    conf->admin_status  = AMB_BGP_ADMIN_STATUS_UP;
    conf->state_kept    = AMB_FALSE;

    // Convert to row_update and send
    pdsa_ctm_send_row_update_common (conf, pdsa_fill_amb_bgp_rm_afi_safi); 

    NBB_TRC_EXIT();
    return;
}

// row_update for bgpNmListenTable
NBB_VOID
pdsa_test_row_update_bgp_nm_listen (pdsa_config_t *conf)
{
    NBB_TRC_ENTRY ("pdsa_test_row_update_bgp_nm_listen");

    // Set params
    conf->oid_len       = AMB_BGP_LIS_OID_LEN;
    conf->data_len      = sizeof (AMB_BGP_NM_LISTEN);
    conf->entity_index  = 1;
    conf->sck_index     = 1;
    conf->admin_status  = AMB_BGP_ADMIN_STATUS_UP;
    conf->row_status    = AMB_ROW_ACTIVE;
    pdsa_convert_long_to_pdsa_ipv4_addr (conf->g_node_a_ip,  &conf->ip_addr);

    // Convert to row_update and send
    pdsa_ctm_send_row_update_common (conf, pdsa_fill_amb_bgp_nm_listen); 

    NBB_TRC_EXIT();
    return;
}

// row_update for bgpRmAfmJoinTable
NBB_VOID
pdsa_test_row_update_bgp_rm_afm_join (pdsa_config_t *conf,
                                      NBB_LONG      join,
                                      NBB_LONG      afi,
                                      NBB_LONG      safi)
{
    NBB_TRC_ENTRY ("pdsa_test_row_update_bgp_rm_afm_join");

    // Set params
    conf->oid_len       = AMB_BGP_AFM_OID_LEN;
    conf->data_len      = sizeof (AMB_BGP_RM_AFM_JOIN);
    conf->entity_index  = 1;
    conf->row_status    = AMB_ROW_ACTIVE;
    conf->join          = join;
    conf->afi           = afi;
    conf->safi          = safi;
    conf->admin_status  = AMB_BGP_ADMIN_STATUS_UP;
    conf->partner_index = 1;

    // Convert to row_update and send
    pdsa_ctm_send_row_update_common (conf, pdsa_fill_amb_bgp_rm_afm_join); 

    NBB_TRC_EXIT();
    return;
}

// row_update for bgpPeerTable
NBB_VOID
pdsa_test_row_update_bgp_peer (pdsa_config_t *conf)
{
    NBB_TRC_ENTRY ("pdsa_test_row_update_bgp_peer");

    // Set params
    conf->oid_len               = AMB_BGP_PER_OID_LEN;
    conf->data_len              = sizeof (AMB_BGP_PEER);
    conf->entity_index          = 1;
    conf->row_status            = AMB_ROW_ACTIVE;
    conf->local_port            = 0;
    conf->remote_port           = 0;
    conf->local_addr_scope_id   = 0;
    conf->connect_retry_interval= 10;
    conf->remote_as             = 1;
    conf->local_nm              = 1;
    conf->config_passive        = AMB_FALSE;
    conf->admin_status          = AMB_BGP_ADMIN_STATUS_UP;
    pdsa_convert_long_to_pdsa_ipv4_addr (conf->g_node_a_ip,  &conf->ip_addr);
    pdsa_convert_long_to_pdsa_ipv4_addr (conf->g_node_b_ip,  &conf->remote_addr);

    // Convert to row_update and send
    pdsa_ctm_send_row_update_common (conf, pdsa_fill_amb_bgp_peer); 

    NBB_TRC_EXIT();
    return;
}

// row_update for bgpPeerAfiSafiTable
NBB_VOID
pdsa_test_row_update_bgp_peer_afi_safi (pdsa_config_t *conf,
                                        NBB_LONG      afi,
                                        NBB_LONG      safi)
{
    NBB_TRC_ENTRY ("pdsa_test_row_update_bgp_peer_afi_safi");

    // Set params
    conf->oid_len               = AMB_BGP_PER_OID_LEN;
    conf->data_len              = sizeof (AMB_BGP_PEER);
    conf->entity_index          = 1;
    conf->local_port            = 0;
    conf->remote_port           = 0;
    conf->local_addr_scope_id   = 0;
    conf->afi                   = afi;
    conf->safi                  = safi;
    pdsa_convert_long_to_pdsa_ipv4_addr (conf->g_node_a_ip,  &conf->ip_addr);
    pdsa_convert_long_to_pdsa_ipv4_addr (conf->g_node_b_ip,  &conf->remote_addr);

    // Convert to row_update and send
    pdsa_ctm_send_row_update_common (conf, pdsa_fill_amb_bgp_peer_afi_safi); 

    NBB_TRC_EXIT();
    return;
}
