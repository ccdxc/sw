// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//Purpose: Helper APIs for metaswitch BGP-RM/NM components

#include "nic/metaswitch/stubs/mgmt/pds_ms_mgmt_utils.hpp"
#include "nic/metaswitch/stubs/pdsa_stubs_init.hpp"
#include "qb0mib.h"

namespace pds {

NBB_VOID 
bgp_peer_fill_func (pds::BGPPeerSpec&   req,
                    AMB_GEN_IPS         *mib_msg,
                    AMB_BGP_PEER        *v_amb_bgp_peer,
                    NBB_LONG            row_status)
{
    // This fill API get called as part of BGP GLobal Spec config after init.
    // Always set admin status to UP
    if (row_status != AMB_ROW_DESTROY) {
        NBB_TRC_FLOW ((NBB_FORMAT "Not destroying peer: fill in field Local_NM"));
        v_amb_bgp_peer->local_nm = PDS_MS_BGP_NM_ENT_INDEX;
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_PER_LOCAL_NM);

        // Trap settings. Enabled by default for all Peers
        v_amb_bgp_peer->trap_estab = AMB_TRUE;
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_PER_TRAP_ESTAB);
        v_amb_bgp_peer->trap_backw = AMB_TRUE;
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_PER_TRAP_BACKW);
    }
}

NBB_VOID 
bgp_rm_ent_fill_func (pds::BGPGlobalSpec &req,
                      AMB_GEN_IPS        *mib_msg,
                      AMB_BGP_RM_ENT     *v_amb_bgp_rm_ent,
                      NBB_LONG           row_status)
{
    // This fill API get called as part of BGP GLobal Spec config after init.
    // Always set admin status to UP
    if (row_status != AMB_ROW_DESTROY) {
        NBB_TRC_FLOW ((NBB_FORMAT "Not destroying RM: fill in field admin_status and I3 in AMB_BGP_RM_ENT"));
        v_amb_bgp_rm_ent->admin_status = AMB_BGP_ADMIN_STATUS_UP; 
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_RM_ADMIN_STATUS);

        v_amb_bgp_rm_ent->i3_ent_index = PDS_MS_I3_ENT_INDEX;;
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_RM_I3_ENT_INDEX);

    }
}
} // end namespace pds

namespace pds_ms_stub {

// Fill bgpRmEntTable: AMB_BGP_RM_ENT
NBB_VOID
pds_ms_fill_amb_bgp_rm (AMB_GEN_IPS *mib_msg, pds_ms_config_t *conf)
{
    // Local variables
    NBB_ULONG       *oid = NULL; 
    AMB_BGP_RM_ENT  *data= NULL;

    NBB_TRC_ENTRY ("pds_ms_fill_amb_bgp_rm");

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
        // RM Entity is always admin down during init
        data->admin_status = AMB_BGP_ADMIN_STATUS_DOWN;
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_RM_ADMIN_STATUS);
    }

    NBB_TRC_EXIT();
    return;
} 

// Fill  bgpNmEntTable: AMB_BGP_NM_ENT
NBB_VOID
pds_ms_fill_amb_bgp_nm (AMB_GEN_IPS *mib_msg, pds_ms_config_t *conf)
{
    // Local variables
    NBB_ULONG       *oid = NULL; 
    AMB_BGP_NM_ENT  *data= NULL;

    NBB_TRC_ENTRY ("pds_ms_fill_amb_bgp_nm");

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
        data->rm_index = PDS_MS_BGP_RM_ENT_INDEX;
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_NM_RM_INDEX);

        data->admin_status = conf->admin_status;
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_NM_ADMIN_STATUS);

        data->sck_index = PDS_MS_SCK_ENT_INDEX;
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_NM_SCK_INDEX);
    }

    NBB_TRC_EXIT();
    return;
}


// Fill bgpRmAfiSafiTable: AMB_BGP_RM_AFI_SAFI
NBB_VOID
pds_ms_fill_amb_bgp_rm_afi_safi (AMB_GEN_IPS *mib_msg, pds_ms_config_t *conf)
{
    // Local variables
    NBB_ULONG           *oid = NULL; 
    AMB_BGP_RM_AFI_SAFI *data= NULL;

    NBB_TRC_ENTRY ("pds_ms_fill_amb_bgp_rm_afi_safi");

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

    oid[AMB_BGP_AFI_AFI_INDEX]  = AMB_BGP_AFI_L2VPN;
    data->afi                   = AMB_BGP_AFI_L2VPN;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_AFI_AFI);

    oid[AMB_BGP_AFI_SAFI_INDEX] = AMB_BGP_EVPN;
    data->safi                  = AMB_BGP_EVPN;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_AFI_SAFI);

    data->next_hop_safi = AMB_BGP_UNICAST;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_AFI_NEXT_HOP_SAFI);

    data->admin_status = AMB_BGP_ADMIN_STATUS_UP;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_AFI_ADMIN_STATUS);

    data->state_kept = AMB_FALSE;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_AFI_STATE_KEPT);

    NBB_TRC_EXIT();
    return;
}

// Fill bgpNmListenTable: AMB_BGP_NM_LISTEN
NBB_VOID
pds_ms_fill_amb_bgp_nm_listen (AMB_GEN_IPS *mib_msg, pds_ms_config_t *conf)
{
    // Local variables
    NBB_ULONG           *oid = NULL; 
    AMB_BGP_NM_LISTEN   *data= NULL;

    NBB_TRC_ENTRY ("pds_ms_fill_amb_bgp_nm_listen");

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

    oid[AMB_BGP_LIS_INDEX_INDEX]    = PDS_MS_SCK_ENT_INDEX;
    data->index                     = PDS_MS_SCK_ENT_INDEX;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_LIS_INDEX);

    data->row_status = conf->row_status;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_LIS_ROW_STATUS);

    if (data->row_status != AMB_ROW_DESTROY)
    {
        NBB_TRC_FLOW((NBB_FORMAT "Not destroying NM LISTEN: fill in fields"));
        data->admin_status = conf->admin_status;
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_LIS_ADMIN_STATUS);

        data->addr_type = AMB_INETWK_ADDR_TYPE_IPV4;
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_LIS_ADDR_TYPE);

        data->addr_len = 0;
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_LIS_ADDR);
    }

    NBB_TRC_EXIT();
    return;
}

// Fill bgpRmAfmJoinTable: AMB_BGP_RM_AFM_JOIN
NBB_VOID
pds_ms_fill_amb_bgp_rm_afm_join (AMB_GEN_IPS *mib_msg, pds_ms_config_t *conf)
{
    // Local variables
    NBB_ULONG           *oid = NULL; 
    AMB_BGP_RM_AFM_JOIN *data= NULL;

    NBB_TRC_ENTRY ("pds_ms_fill_amb_bgp_rm_afm_join");

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

    oid[AMB_BGP_AFM_JOIN_INDEX] = conf->join_index;
    data->join                  = conf->join_index;
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

// row_update for BGP-RM
NBB_VOID
pds_ms_row_update_bgp_rm (pds_ms_config_t *conf)
{
    NBB_TRC_ENTRY ("pds_ms_row_update_bgp_rm");

    // Set params
    conf->oid_len           = AMB_BGP_RM_OID_LEN;
    conf->data_len          = sizeof (AMB_BGP_RM_ENT);

    // Convert to row_update and send
    pds_ms_ctm_send_row_update_common (conf, pds_ms_fill_amb_bgp_rm); 

    NBB_TRC_EXIT();
    return;
}

// row_update for BGP-NM
NBB_VOID
pds_ms_row_update_bgp_nm (pds_ms_config_t *conf)
{
    NBB_TRC_ENTRY ("pds_ms_row_update_bgp_nm");

    // Set params
    conf->oid_len       = AMB_BGP_NM_OID_LEN;
    conf->data_len      = sizeof (AMB_BGP_NM_ENT);

    // Convert to row_update and send
    pds_ms_ctm_send_row_update_common (conf, pds_ms_fill_amb_bgp_nm); 

    NBB_TRC_EXIT();
    return;
}

// row_update for bgpRmAfiSafiTable
NBB_VOID
pds_ms_row_update_bgp_rm_afi_safi (pds_ms_config_t *conf)
{
    NBB_TRC_ENTRY ("pds_ms_row_update_bgp_rm_afi_safi");

    // Set params
    conf->oid_len       = AMB_BGP_AFI_OID_LEN;
    conf->data_len      = sizeof (AMB_BGP_RM_AFI_SAFI);

    // Convert to row_update and send
    pds_ms_ctm_send_row_update_common (conf, pds_ms_fill_amb_bgp_rm_afi_safi); 

    NBB_TRC_EXIT();
    return;
}

// row_update for bgpNmListenTable
NBB_VOID
pds_ms_row_update_bgp_nm_listen (pds_ms_config_t *conf)
{
    NBB_TRC_ENTRY ("pds_ms_row_update_bgp_nm_listen");

    // Set params
    conf->oid_len       = AMB_BGP_LIS_OID_LEN;
    conf->data_len      = sizeof (AMB_BGP_NM_LISTEN);

    // Convert to row_update and send
    pds_ms_ctm_send_row_update_common (conf, pds_ms_fill_amb_bgp_nm_listen); 

    NBB_TRC_EXIT();
    return;
}

// row_update for bgpRmAfmJoinTable
NBB_VOID
pds_ms_row_update_bgp_rm_afm_join (pds_ms_config_t *conf)
{
    NBB_TRC_ENTRY ("pds_ms_row_update_bgp_rm_afm_join");

    // Set params
    conf->oid_len       = AMB_BGP_AFM_OID_LEN;
    conf->data_len      = sizeof (AMB_BGP_RM_AFM_JOIN);

    // Convert to row_update and send
    pds_ms_ctm_send_row_update_common (conf, pds_ms_fill_amb_bgp_rm_afm_join); 

    NBB_TRC_EXIT();
    return;
}

NBB_VOID
pds_ms_bgp_create (pds_ms_config_t *conf)
{
    NBB_TRC_ENTRY ("pds_ms_bgp_create");

   // bgpRmEntTable
   conf->entity_index        = PDS_MS_BGP_RM_ENT_INDEX;
   conf->admin_status        = AMB_ADMIN_STATUS_DOWN;
   pds_ms_row_update_bgp_rm (conf);

    // bgpNmEntTable
    conf->entity_index       = PDS_MS_BGP_NM_ENT_INDEX;
    conf->admin_status       = AMB_ADMIN_STATUS_UP;
    pds_ms_row_update_bgp_nm (conf);

    // bgpRmAfmJoinTable
    conf->entity_index       = PDS_MS_BGP_RM_ENT_INDEX;
    conf->admin_status       = AMB_ADMIN_STATUS_UP;
    conf->partner_index      = 1;

    // AFM: AMB_BGP_AFI_IPV4
    conf->join_index         = 1;
    conf->afi                = AMB_BGP_AFI_IPV4;
    conf->safi               = AMB_BGP_UNICAST;
    pds_ms_row_update_bgp_rm_afm_join (conf);

    // AFM: AMB_BGP_AFI_L2VPN
    conf->join_index         = 2;
    conf->afi                = AMB_BGP_AFI_L2VPN;
    conf->safi               = AMB_BGP_EVPN;
    pds_ms_row_update_bgp_rm_afm_join (conf);

    // bgpRmAfiSafiTable
    conf->entity_index      = 1;
    pds_ms_row_update_bgp_rm_afi_safi (conf);

    // bgpNmListenTable
    conf->entity_index       = PDS_MS_BGP_NM_ENT_INDEX;
    conf->admin_status       = AMB_ADMIN_STATUS_UP;
    pds_ms_row_update_bgp_nm_listen (conf);

    NBB_TRC_EXIT();
    return;
}
}
