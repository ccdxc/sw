// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//Purpose: Helper APIs for metaswitch BGP-RM/NM components

#include "pdsa_mgmt_utils.hpp"
#include "qb0mib.h"


// Fill bgpRmEntTable: AMB_BGP_RM_ENT
NBB_VOID
pdsa_fill_amb_bgp_rm (AMB_GEN_IPS       *mib_msg,
                      AMB_BGP_RM_ENT    *v_amb_bgp_rm,
                      NBB_ULONG         index,
                      NBB_ULONG         i3_ent_index,
                      NBB_LONG          row_status,
                      NBB_LONG          restart_supported,
                      NBB_ULONG         local_as,
                      NBB_LONG          admin_status,
                      NBB_LONG          router_id) 
{
  NBB_TRC_ENTRY ("pdsa_fill_amb_bgp_rm");

  // Set all fields absent
  AMB_SET_ALL_FIELDS_NOT_PRESENT (mib_msg);

  // Set all incoming fields
  v_amb_bgp_rm->index = index;
  AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_RM_INDEX);

  v_amb_bgp_rm->row_status = row_status;
  AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_RM_ROW_STATUS);

  if (row_status != AMB_ROW_DESTROY)
  {
      NBB_TRC_FLOW ((NBB_FORMAT "Not destroying RM: fill in fields"));
      v_amb_bgp_rm->admin_status = admin_status;
      AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_RM_ADMIN_STATUS);

      v_amb_bgp_rm->local_as = local_as;
      AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_RM_LOCAL_AS);

      NBB_PUT_LONG(v_amb_bgp_rm->local_identifier, router_id);
      AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_RM_LOCAL_IDENTIFIR);

      v_amb_bgp_rm->i3_ent_index = i3_ent_index;
      AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_RM_I3_ENT_INDEX);

      v_amb_bgp_rm->restart_supported = restart_supported;
      AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_RM_RESTART_SUPP);
  }

  NBB_TRC_EXIT();
  return;
} 

// Fill  bgpNmEntTable: AMB_BGP_NM_ENT
NBB_VOID
pdsa_fill_amb_bgp_nm (AMB_GEN_IPS       *mib_msg,
                      AMB_BGP_NM_ENT    *v_amb_bgp_nm,
                      NBB_ULONG         index,
                      NBB_LONG          row_status,
                      NBB_LONG          admin_status,
                      NBB_ULONG         sck_index,
                      NBB_ULONG         rm_index)
{
  NBB_TRC_ENTRY ("pdsa_fill_amb_bgp_nm");

  // Set all fields absent
  AMB_SET_ALL_FIELDS_NOT_PRESENT (mib_msg);

  // Set all incoming fields
  v_amb_bgp_nm->index = index;
  AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_NM_INDEX);

  v_amb_bgp_nm->row_status = row_status;
  AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_NM_ROW_STATUS);

  v_amb_bgp_nm->rm_index = rm_index;
  AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_NM_RM_INDEX);

  if (row_status != AMB_ROW_DESTROY)
  {
      NBB_TRC_FLOW ((NBB_FORMAT "Not destroying NM: fill in fields"));
      v_amb_bgp_nm->admin_status = admin_status;
      AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_NM_ADMIN_STATUS);

      v_amb_bgp_nm->sck_index = sck_index;
      AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_NM_SCK_INDEX);
  }

  NBB_TRC_EXIT();
  return;
}


// Fill bgpRmAfiSafiTable: AMB_BGP_RM_AFI_SAFI
NBB_VOID
pdsa_fill_amb_bgp_rm_afi_safi (AMB_GEN_IPS          *mib_msg,
                               AMB_BGP_RM_AFI_SAFI  *v_amb_bgp_rm_afi_safi,
                               NBB_ULONG            ent_index,
                               NBB_LONG             afi,
                               NBB_LONG             safi,
                               NBB_LONG             next_hop_safi,
                               NBB_LONG             admin_status,
                               NBB_LONG             state_kept)
{
    NBB_TRC_ENTRY ("pdsa_fill_amb_bgp_rm_afi_safi");

    // Set all fields absent
    AMB_SET_ALL_FIELDS_NOT_PRESENT (mib_msg);

    // Set all incoming fields
    v_amb_bgp_rm_afi_safi->ent_index = ent_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_AFI_ENT_INDEX);

    v_amb_bgp_rm_afi_safi->afi = afi;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_AFI_AFI);

    v_amb_bgp_rm_afi_safi->safi = safi;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_AFI_SAFI);

    v_amb_bgp_rm_afi_safi->next_hop_safi = next_hop_safi;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_AFI_NEXT_HOP_SAFI);

    v_amb_bgp_rm_afi_safi->admin_status = admin_status;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_AFI_ADMIN_STATUS);

    v_amb_bgp_rm_afi_safi->state_kept = state_kept;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_AFI_STATE_KEPT);

    NBB_TRC_EXIT();
    return;
}


// Fill bgpNmListenTable: AMB_BGP_NM_LISTEN
NBB_VOID
pdsa_fill_amb_bgp_nm_listen (AMB_GEN_IPS        *mib_msg,
                             AMB_BGP_NM_LISTEN  *v_amb_bgp_nm_listen,
                             NBB_ULONG          ent_index,
                             NBB_ULONG          index,
                             NBB_LONG           admin_status,
                             ip_addr_t          ip_addr,
                             NBB_LONG           row_status)
{
    NBB_TRC_ENTRY ("pdsa_fill_amb_bgp_nm_listen");

    // Set all fields absent
    AMB_SET_ALL_FIELDS_NOT_PRESENT (mib_msg);

    // Set all incoming fields
    v_amb_bgp_nm_listen->ent_index = ent_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_LIS_ENT_INDEX);

    v_amb_bgp_nm_listen->index = index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_LIS_INDEX);

    v_amb_bgp_nm_listen->row_status = row_status;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_LIS_ROW_STATUS);

    if (row_status != AMB_ROW_DESTROY)
    {
        NBB_TRC_FLOW((NBB_FORMAT "Not destroying NM LISTEN: fill in fields"));
        v_amb_bgp_nm_listen->admin_status = admin_status;
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_LIS_ADMIN_STATUS);

        pdsa_convert_ip_addr_to_amb_ip_addr(ip_addr, 
                                            &v_amb_bgp_nm_listen->addr_type,
                                            &v_amb_bgp_nm_listen->addr_len,
                                            v_amb_bgp_nm_listen->addr);
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_LIS_ADDR_TYPE);
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_LIS_ADDR);
    }

    NBB_TRC_EXIT();
    return;
}

// Fill bgpRmAfmJoinTable: AMB_BGP_RM_AFM_JOIN
NBB_VOID
pdsa_fill_amb_bgp_rm_afm_join (AMB_GEN_IPS          *mib_msg,
                               AMB_BGP_RM_AFM_JOIN  *v_amb_bgp_rm_afm_join,
                               NBB_ULONG            ent_index,
                               NBB_ULONG            join,
                               NBB_LONG             afi,
                               NBB_LONG             safi,
                               NBB_LONG             admin_status,
                               NBB_ULONG            partner_index,
                               NBB_LONG             row_status)
{
    NBB_TRC_ENTRY ("pdsa_fill_amb_bgp_rm_afm_join");

    // Set all fields absent
    AMB_SET_ALL_FIELDS_NOT_PRESENT (mib_msg);

    // Set all incoming fields
    v_amb_bgp_rm_afm_join->ent_index = ent_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_AFM_ENTITY);

    v_amb_bgp_rm_afm_join->join = join;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_AFM_JOIN);

    v_amb_bgp_rm_afm_join->row_status = row_status;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_AFM_ROW_STATUS);

    if (row_status != AMB_ROW_DESTROY)
    {
        NBB_TRC_FLOW((NBB_FORMAT "Not destroying RM AFM Join: fill in fields"));
        v_amb_bgp_rm_afm_join->admin_status = admin_status;
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_AFM_ADMIN_STATUS);

        v_amb_bgp_rm_afm_join->afi = afi;
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_AFM_AFI);

        v_amb_bgp_rm_afm_join->safi = safi;
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_AFM_SAFI);

        v_amb_bgp_rm_afm_join->partner_index = partner_index;
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_AFM_PARTNER_INDEX);
    }

    NBB_TRC_EXIT();
    return;
}

// Fill bgpPeerTable: AMB_BGP_PEER
NBB_VOID
pdsa_fill_amb_bgp_peer (AMB_GEN_IPS     *mib_msg,
                        AMB_BGP_PEER    *v_amb_bgp_peer,
                        NBB_ULONG       rm_ent_index,
                        ip_addr_t       local_addr,
                        NBB_ULONG       local_port,
                        ip_addr_t       remote_addr,
                        NBB_ULONG       remote_port,
                        NBB_ULONG       local_addr_scope_id,
                        NBB_ULONG       connect_retry_interval,
                        NBB_ULONG       remote_as,
                        NBB_ULONG       local_nm,
                        NBB_LONG        config_passive,
                        NBB_LONG        admin_status,
                        NBB_LONG        row_status)
{
    NBB_TRC_ENTRY ("pdsa_fill_amb_bgp_peer");

    // Set all fields absent
    AMB_SET_ALL_FIELDS_NOT_PRESENT (mib_msg);

    // Set all incoming fields
    v_amb_bgp_peer->rm_ent_index = rm_ent_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_PER_RM_ENT_INDEX);

    pdsa_convert_ip_addr_to_amb_ip_addr(local_addr, 
                                        &v_amb_bgp_peer->local_addr_type,
                                        &v_amb_bgp_peer->local_addr_len,
                                        v_amb_bgp_peer->local_addr);
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_PER_LOCAL_ADDR_TYP);
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_PER_LOCAL_ADDR);

    v_amb_bgp_peer->local_port = local_port;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_PER_LOCAL_PORT);

    pdsa_convert_ip_addr_to_amb_ip_addr(remote_addr, 
                                        &v_amb_bgp_peer->remote_addr_type,
                                        &v_amb_bgp_peer->remote_addr_len,
                                        v_amb_bgp_peer->remote_addr);
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_PER_REMOTE_DDR_TYP);
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_PER_REMOTE_ADDR);

    v_amb_bgp_peer->remote_port = remote_port;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_PER_REMOTE_PORT);

    v_amb_bgp_peer->local_addr_scope_id = local_addr_scope_id;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_PER_LOC_ADDR_SCOPE);

    v_amb_bgp_peer->row_status = row_status;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_PER_ROW_STATUS);

    if (row_status != AMB_ROW_DESTROY)
    {
        NBB_TRC_FLOW((NBB_FORMAT "Not destroying BGP PEER: fill in fields"));
        v_amb_bgp_peer->connect_retry_interval = connect_retry_interval;
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_PER_CNNCT_RTRY_NTR);

        v_amb_bgp_peer->remote_as = remote_as;
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_PER_REMOTE_AS);

        v_amb_bgp_peer->config_passive = config_passive;
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_PER_CFG_PASSIVE);

        v_amb_bgp_peer->admin_status = admin_status;
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_PER_ADMIN_STATUS);

        v_amb_bgp_peer->local_nm = local_nm;
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_PER_LOCAL_NM);
    }

    NBB_TRC_EXIT();
    return;
}


// Fill bgpPeerTable: AMB_BGP_PEER_AFI_SAFI
NBB_VOID
pdsa_fill_amb_bgp_peer_afi_safi (AMB_GEN_IPS            *mib_msg,
                                AMB_BGP_PEER_AFI_SAFI   *v_amb_bgp_peer_af,
                                NBB_ULONG               rm_ent_index,
                                ip_addr_t               local_addr,
                                NBB_ULONG               local_port,
                                ip_addr_t               remote_addr,
                                NBB_ULONG               remote_port,
                                NBB_ULONG               local_addr_scope_id,
                                NBB_LONG                afi,
                                NBB_LONG                safi,
                                NBB_LONG                disable_afi_safi)
{
    NBB_TRC_ENTRY ("pdsa_fill_amb_bgp_peer_afi_safi");

    // Set all fields absent
    AMB_SET_ALL_FIELDS_NOT_PRESENT (mib_msg);

    // Set all incoming fields
    v_amb_bgp_peer_af->rm_ent_index = rm_ent_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_PAS_RM_ENT_INDEX);

    pdsa_convert_ip_addr_to_amb_ip_addr(local_addr, 
                                        &v_amb_bgp_peer_af->local_addr_type,
                                        &v_amb_bgp_peer_af->local_addr_len,
                                        v_amb_bgp_peer_af->local_addr);
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_PAS_LOCAL_ADDR_TYP);
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_PAS_LOCAL_ADDR);

    v_amb_bgp_peer_af->local_port = local_port;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_PER_LOCAL_PORT);

    pdsa_convert_ip_addr_to_amb_ip_addr(remote_addr, 
                                        &v_amb_bgp_peer_af->remote_addr_type,
                                        &v_amb_bgp_peer_af->remote_addr_len,
                                        v_amb_bgp_peer_af->remote_addr);
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_PAS_REMOTE_ADDR_TYP);
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_PAS_REMOTE_ADDR);

    v_amb_bgp_peer_af->remote_port = remote_port;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_PAS_REMOTE_PORT);

    v_amb_bgp_peer_af->local_addr_scope_id = local_addr_scope_id;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_PAS_SCOPE);

    v_amb_bgp_peer_af->afi = afi;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_PAS_AFI);

    v_amb_bgp_peer_af->safi = safi;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_PAS_SAFI);

    v_amb_bgp_peer_af->disable_afi_safi= disable_afi_safi;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_PAS_DISABLE);

    //this table doesnt have row_status
    NBB_TRC_EXIT();
    return;
}
