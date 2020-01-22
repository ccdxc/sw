// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//Purpose: Helper APIs for metaswitch BGP-RM/NM components

#include "nic/metaswitch/stubs/mgmt/pds_ms_uuid_obj.hpp"
#include "nic/metaswitch/stubs/mgmt/pds_ms_mgmt_state.hpp"
#include "nic/metaswitch/stubs/mgmt/pds_ms_mgmt_utils.hpp"
#include "nic/metaswitch/stubs/pds_ms_stubs_init.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_util.hpp"
#include "qb0mib.h"
using namespace pds_ms;

namespace pds {

static NBB_VOID 
bgp_peer_fill_keys_(pds::BGPPeerSpec& req, bgp_peer_uuid_obj_t* uuid_obj)
{
    auto bgp_peer_uuid_obj = (bgp_peer_uuid_obj_t*) uuid_obj;
    auto localaddr = req.mutable_localaddr();
    ip_addr_to_spec(&bgp_peer_uuid_obj->ms_id().local_ip, localaddr);

    auto peeraddr = req.mutable_peeraddr();
    ip_addr_to_spec(&bgp_peer_uuid_obj->ms_id().peer_ip, peeraddr);

    SDK_TRACE_VERBOSE("BGP Peer Pre-set Keys UUID %s Local IP %s Peer IP %s",
                      uuid_obj->uuid().str(),
                      ipaddr2str(&bgp_peer_uuid_obj->ms_id().local_ip),
                      ipaddr2str(&bgp_peer_uuid_obj->ms_id().peer_ip));
}

static NBB_VOID 
bgp_peer_af_fill_keys_(pds::BGPPeerAf& req,
                       bgp_peer_af_uuid_obj_t* uuid_obj)
{
    auto bgp_peer_af_uuid_obj = (bgp_peer_af_uuid_obj_t*) uuid_obj;
    auto localaddr = req.mutable_localaddr();
    ip_addr_to_spec(&bgp_peer_af_uuid_obj->ms_id().local_ip, localaddr);

    auto peeraddr = req.mutable_peeraddr();
    ip_addr_to_spec(&bgp_peer_af_uuid_obj->ms_id().peer_ip, peeraddr);

    SDK_TRACE_VERBOSE("BGP Peer Pre-set Keys UUID %s Local IP %s Peer IP %s",
                      uuid_obj->uuid().str(),
                      ipaddr2str(&bgp_peer_af_uuid_obj->ms_id().local_ip),
                      ipaddr2str(&bgp_peer_af_uuid_obj->ms_id().peer_ip));
}

NBB_VOID
bgp_peer_pre_set(pds::BGPPeerSpec &req, NBB_LONG row_status, NBB_ULONG correlator) 
{
    pds_obj_key_t uuid = {0};
    pds_ms_get_uuid(&uuid, req.id());

    auto mgmt_ctxt = mgmt_state_t::thread_context();
    auto uuid_obj = mgmt_ctxt.state()->lookup_uuid(uuid);

    if (uuid_obj == nullptr) {
        if (row_status == AMB_ROW_DESTROY) {
            SDK_TRACE_ERR("BGP Peer delete with unknown key %s", uuid.str());
            return;
        }
        // BGP Peer Create - store UUID to key mapping pending confirmation
        ip_addr_t local_ipaddr, peer_ipaddr;
        ip_addr_spec_to_ip_addr (req.localaddr(), &local_ipaddr);
        ip_addr_spec_to_ip_addr (req.peeraddr(), &peer_ipaddr);
        bgp_peer_uuid_obj_uptr_t bgp_peer_uuid_obj 
            (new bgp_peer_uuid_obj_t (uuid,
                                      local_ipaddr,
                                      peer_ipaddr));
        {
            mgmt_ctxt.state()->set_pending_uuid_create(uuid,
                                                       std::move(bgp_peer_uuid_obj));
        }
        SDK_TRACE_VERBOSE("BGP Peer Pre-set Create UUID %s Local IP %s Peer IP %s",
                          uuid.str(), ipaddr2str(&local_ipaddr),
                          ipaddr2str(&peer_ipaddr));
    }
    else if (uuid_obj->obj_type() == uuid_obj_type_t::BGP_PEER) {
        // BGP Peer Update - Fill keys
        bgp_peer_fill_keys_(req, (bgp_peer_uuid_obj_t*)uuid_obj);
        if (row_status == AMB_ROW_DESTROY) {
            mgmt_ctxt.state()->set_pending_uuid_delete(uuid);
        }
    } else {
        // Venice may use the same UUID for multiple protos
        // BGP Global, Peer and PeerAF
        // in which case it will fill the appropriate keys
        // and the UUID need not be created or deleted
        SDK_TRACE_VERBOSE("Received BGP Peer request with UUID type %s",
                          uuid_obj_type_str(uuid_obj->obj_type()));
    }
}

NBB_VOID
bgp_peer_afi_safi_pre_set(pds::BGPPeerAf &req, NBB_LONG row_status,
                          NBB_ULONG correlator) 
{
    pds_obj_key_t uuid = {0};
    pds_ms_get_uuid(&uuid, req.id());

    auto mgmt_ctxt = mgmt_state_t::thread_context();
    auto uuid_obj = mgmt_ctxt.state()->lookup_uuid(uuid);

    if (uuid_obj == nullptr) {
        if (row_status == AMB_ROW_DESTROY) {
            SDK_TRACE_ERR("BGP PeerAF delete with unknown key %s", uuid.str());
            return;
        }
        // BGP PeerAF Create - store UUID to key mapping pending confirmation
        ip_addr_t local_ipaddr, peer_ipaddr;
        ip_addr_spec_to_ip_addr (req.localaddr(), &local_ipaddr);
        ip_addr_spec_to_ip_addr (req.peeraddr(), &peer_ipaddr);
        bgp_peer_af_uuid_obj_uptr_t bgp_peer_af_uuid_obj 
            (new bgp_peer_af_uuid_obj_t (uuid, local_ipaddr, peer_ipaddr,
                                         req.afi(), req.safi()));
        {
            mgmt_ctxt.state()->set_pending_uuid_create(uuid,
                                                       std::move(bgp_peer_af_uuid_obj));
        }
        SDK_TRACE_VERBOSE("BGP PeerAF Pre-set Create UUID %s Local IP %s "
                          "Peer IP %s AFI %d SAFI %d",
                          uuid.str(), ipaddr2str(&local_ipaddr),
                          ipaddr2str(&peer_ipaddr), req.afi(), req.safi());
    } else if (uuid_obj->obj_type() == uuid_obj_type_t::BGP_PEER_AF) {
        // BGP Peer Update - Fill keys
        bgp_peer_af_fill_keys_(req, (bgp_peer_af_uuid_obj_t*)uuid_obj);
        if (row_status == AMB_ROW_DESTROY) {
            mgmt_ctxt.state()->set_pending_uuid_delete(uuid);
        }
    } else {
        // Venice may use the same UUID for multiple protos
        // BGP Global, Peer and PeerAF
        // in which case it will fill the appropriate keys
        // and the UUID need not be created or deleted
        SDK_TRACE_VERBOSE("Received BGP PeerAF request with UUID type %s",
                          uuid_obj_type_str(uuid_obj->obj_type()));
    }
}

NBB_VOID 
bgp_peer_fill_func (pds::BGPPeerSpec&   req,
                    AMB_GEN_IPS         *mib_msg,
                    AMB_BGP_PEER        *v_amb_bgp_peer,
                    NBB_LONG            row_status)
{
    // Local variables
    NBB_ULONG *oid = (NBB_ULONG *)((NBB_BYTE *)mib_msg + mib_msg->oid_offset);

    // ASSUMPTION - There is only a single BGP instance ever.
    // So the BGP instance UUID is not included in the BGP Peer Proto. 
    // Harcoding the BGP entity index.
    v_amb_bgp_peer->rm_ent_index        = PDS_MS_BGP_RM_ENT_INDEX;
    oid[AMB_BGP_PER_RM_ENT_INDEX_INDEX] = v_amb_bgp_peer->rm_ent_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_PER_RM_ENT_INDEX);

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

        // Avoid delay in advertising Type-2 withdrawal for all
        // IP addresses associated with a MAC in case of the EVPN Remote
        // to Local MAC/IP move scenario since it causes spurious IP add
        // for the invalid IP addresses
        v_amb_bgp_peer->min_asorigination_interval = 1;
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_PER_MIN_AS_ORIG_INT);
        v_amb_bgp_peer->min_route_advertise_interval = 1;
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_PEER_MIN_RT_ADVERT);
        v_amb_bgp_peer->min_route_withdraw_interval = 1;
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_PEER_MIN_RT_WDRW);
    }
}

NBB_VOID 
bgp_rm_ent_fill_func (pds::BGPGlobalSpec &req,
                      AMB_GEN_IPS        *mib_msg,
                      AMB_BGP_RM_ENT     *v_amb_bgp_rm_ent,
                      NBB_LONG           row_status)
{
    // Local variables
    NBB_ULONG *oid = (NBB_ULONG *)((NBB_BYTE *)mib_msg + mib_msg->oid_offset);

    pds_obj_key_t uuid = {0};
    bgp_uuid_obj_t::ms_id_t entity_index = 0;
    pds_ms_get_uuid(&uuid, req.id());
    {
        auto mgmt_ctxt = mgmt_state_t::thread_context();
        auto uuid_obj = mgmt_ctxt.state()->lookup_uuid(uuid);
        if (uuid_obj == nullptr) {
            bgp_uuid_obj_uptr_t bgp_uuid_obj(new bgp_uuid_obj_t (uuid));
            entity_index = bgp_uuid_obj->ms_id();
            mgmt_ctxt.state()->set_pending_uuid_create(uuid,
                                                       std::move(bgp_uuid_obj));
            SDK_TRACE_VERBOSE("BGP RM Pre-set Create UUID %s Entity %d",
                              uuid.str(), entity_index);
        } else if (uuid_obj->obj_type() == uuid_obj_type_t::BGP) {
            auto bgp_uuid_obj = (bgp_uuid_obj_t*)uuid_obj;
            entity_index = bgp_uuid_obj->ms_id();
        } else {
            SDK_TRACE_ERR("BGP RM Request with unknown UUID %s of type %d",
                          uuid.str(), uuid_obj_type_str(uuid_obj->obj_type()));
        }
    }

    // ASSUMPTION - There is only a single BGP instance ever.
    // Harcoding the BGP entity index.
    v_amb_bgp_rm_ent->index     = entity_index;
    oid[AMB_BGP_RM_INDEX_INDEX] = v_amb_bgp_rm_ent->index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_RM_INDEX);

    // Always set admin status to UP
    if (row_status != AMB_ROW_DESTROY) {
        NBB_TRC_FLOW ((NBB_FORMAT "Not destroying RM: fill in field admin_status and I3 in AMB_BGP_RM_ENT"));
        v_amb_bgp_rm_ent->admin_status = AMB_BGP_ADMIN_STATUS_UP; 
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_RM_ADMIN_STATUS);

        v_amb_bgp_rm_ent->i3_ent_index = PDS_MS_I3_ENT_INDEX;;
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_RM_I3_ENT_INDEX);

    }
}

NBB_VOID 
bgp_peer_af_fill_func (pds::BGPPeerAf        &req,
                       AMB_GEN_IPS           *mib_msg,
                       AMB_BGP_PEER_AFI_SAFI *v_amb_bgp_peer_af,
                       NBB_LONG               row_status)
{
    // Local variables
    NBB_ULONG *oid = (NBB_ULONG *)((NBB_BYTE *)mib_msg + mib_msg->oid_offset);

    // ASSUMPTION - There is only a single BGP instance ever.
    // So the BGP instance UUID is not included in the BGP Peer Proto. 
    // Harcoding the BGP entity index.
    v_amb_bgp_peer_af->rm_ent_index        = PDS_MS_BGP_RM_ENT_INDEX;
    oid[AMB_BGP_PAS_RM_ENT_INDEX_INDEX] = v_amb_bgp_peer_af->rm_ent_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_PAS_RM_ENT_INDEX);
}
} // end namespace pds

namespace pds_ms {

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

        data->max_ibgp_ecmp_routes = 2;
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_RM_MAX_INT_ECMP_RTS);      

        // Enable ORF support
        data->orf_supported  = AMB_TRUE;
        AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_RM_ORF_SUPPORTED);
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

// Fill bgpOrfCapabilityTable: AMB_BGP_ORF_CAP
NBB_VOID
pds_ms_fill_amb_bgp_orf_cap (AMB_GEN_IPS *mib_msg, pds_ms_config_t *conf)
{
    // Local variables
    NBB_ULONG       *oid = NULL; 
    AMB_BGP_ORF_CAP *data= NULL;

    NBB_TRC_ENTRY ("pds_ms_fill_amb_bgp_orf_cap");

    // Get oid and data offset 
    oid     = (NBB_ULONG *)((NBB_BYTE *)mib_msg + mib_msg->oid_offset);
    data    = (AMB_BGP_ORF_CAP *)((NBB_BYTE *)mib_msg + mib_msg->data_offset); 

    // Set all fields absent
    AMB_SET_ALL_FIELDS_NOT_PRESENT (mib_msg);

    // Set OID len and family
    oid[0] = AMB_BGP_ORF_OID_LEN;
    oid[1] = AMB_FAM_BGP_ORF_CAP;

    // Set all incoming fields
    oid[AMB_BGP_ORF_CAP_ENT_INDEX_INDEX] = conf->entity_index;
    data->ent_index                      = conf->entity_index;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_ORF_CAP_ENT_INDEX);

    oid[AMB_BGP_ORF_CAP_AFI_INDEX] = conf->afi;
    data->afi                      = conf->afi;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_ORF_CAP_AFI);

    oid [AMB_BGP_ORF_CAP_SAFI_INDEX] = conf->safi;
    data->safi                       = conf->safi;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_ORF_CAP_SAFI);

    oid[AMB_BGP_ORF_CAP_ORF_TYPE_INDEX] = AMB_BGP_ORF_CAP_TYPE_PREFIX;
    data->orf_type                      = AMB_BGP_ORF_CAP_TYPE_PREFIX;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_ORF_CAP_ORF_TYPE);

    data->admin_status = AMB_BGP_ADMIN_STATUS_UP;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_ORF_CAP_ADMIN_STAT);

    data->send_receive = AMB_BGP_ORF_CAP_SR_BOTH;
    AMB_SET_FIELD_PRESENT (mib_msg, AMB_OID_BGP_ORF_CAP_SEND_RECV);

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

// row_update for bgpOrfCapabilityTable
NBB_VOID
pds_ms_row_update_bgp_orf_cap (pds_ms_config_t *conf)
{
    NBB_TRC_ENTRY ("pds_ms_row_update_bgp_orf_cap");

    // Set params
    conf->oid_len       = AMB_BGP_ORF_OID_LEN;
    conf->data_len      = sizeof (AMB_BGP_ORF_CAP);

    // Convert to row_update and send
    pds_ms_ctm_send_row_update_common (conf, pds_ms_fill_amb_bgp_orf_cap); 

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
    conf->partner_index      = PDS_MS_RTM_DEF_ENT_INDEX;

    // AFM: AMB_BGP_AFI_IPV4
    conf->join_index         = 1;
    conf->afi                = AMB_BGP_AFI_IPV4;
    conf->safi               = AMB_BGP_UNICAST;
    pds_ms_row_update_bgp_rm_afm_join (conf);

    // AFM: AMB_BGP_AFI_L2VPN
    conf->join_index         = 2;
    conf->afi                = AMB_BGP_AFI_L2VPN;
    conf->safi               = AMB_BGP_EVPN;
    conf->partner_index      = PDS_MS_EVPN_ENT_INDEX;
    pds_ms_row_update_bgp_rm_afm_join (conf);
    // TODO: Enable ORF cap on L2VPN AF
    // pds_ms_row_update_bgp_orf_cap (conf);

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
