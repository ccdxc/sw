//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// athena flow session rewrite implementation
///
//----------------------------------------------------------------------------

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/lib/p4/p4_api.hpp"
#include "nic/sdk/lib/p4/p4_utils.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/api/include/athena/pds_flow_session_rewrite.h"
#include "gen/p4gen/athena/include/p4pd.h"

using namespace sdk;

extern "C" {

sdk_ret_t
pds_flow_session_rewrite_create (pds_flow_session_rewrite_spec_t *spec)
{
    p4pd_error_t p4pd_ret;
    uint32_t session_rewrite_id;
    session_rewrite_actiondata_t session_rewrite_actiondata = { 0 };
    session_rewrite_encap_actiondata_t session_encap_actiondata = { 0 };

    if (!spec) {
        PDS_TRACE_ERR("spec is null");
        return SDK_RET_INVALID_ARG;
    }
    session_rewrite_id = spec->key.session_rewrite_id;
    if ((session_rewrite_id == 0) ||
        (session_rewrite_id >= PDS_FLOW_SESSION_REWRITE_ID_MAX)) {
        PDS_TRACE_ERR("session id %u is invalid", session_rewrite_id);
        return SDK_RET_INVALID_ARG;
    }
    if ((spec->data.nat_info.nat_type < REWRITE_NAT_TYPE_NONE) ||
        (spec->data.nat_info.nat_type >= REWRITE_NAT_TYPE_MAX)) {
        PDS_TRACE_ERR("Invalid NAT type %u", spec->data.nat_info.nat_type);
        return SDK_RET_INVALID_ARG;
    }
    if ((spec->data.encap_type < ENCAP_TYPE_NONE) ||
        (spec->data.encap_type >= ENCAP_TYPE_MAX)) {
        PDS_TRACE_ERR("Invalid encap type %u", spec->data.encap_type);
        return SDK_RET_INVALID_ARG;
    }

    // Session rewrite NAT table programming
    if (spec->data.nat_info.nat_type == REWRITE_NAT_TYPE_NONE) {
        session_rewrite_session_rewrite_t *session_rewrite;
        session_rewrite_actiondata.action_id =
            SESSION_REWRITE_SESSION_REWRITE_ID;
        session_rewrite =
            &session_rewrite_actiondata.action_u.session_rewrite_session_rewrite;
        session_rewrite->strip_outer_encap_flag = 
                spec->data.strip_encap_header;
        session_rewrite->strip_l2_header_flag = 
            spec->data.strip_l2_header;
        session_rewrite->strip_vlan_tag_flag = 
            spec->data.strip_vlan_tag;
        session_rewrite->valid_flag = 1;
    } else if (spec->data.nat_info.nat_type == REWRITE_NAT_TYPE_IPV4_SNAT) {
        session_rewrite_session_rewrite_ipv4_snat_t *session_rewrite_ipv4_snat;
        session_rewrite_actiondata.action_id =
            SESSION_REWRITE_SESSION_REWRITE_IPV4_SNAT_ID;
        session_rewrite_ipv4_snat =
            &session_rewrite_actiondata.action_u.session_rewrite_session_rewrite_ipv4_snat;
        session_rewrite_ipv4_snat->strip_outer_encap_flag = 
                spec->data.strip_encap_header;
        session_rewrite_ipv4_snat->strip_l2_header_flag = 
            spec->data.strip_l2_header;
        session_rewrite_ipv4_snat->strip_vlan_tag_flag = 
            spec->data.strip_vlan_tag;
        session_rewrite_ipv4_snat->ipv4_addr_snat =
               spec->data.nat_info.u.ipv4_addr;
        session_rewrite_ipv4_snat->valid_flag = 1;
    } else if (spec->data.nat_info.nat_type == REWRITE_NAT_TYPE_IPV4_DNAT) {
        session_rewrite_session_rewrite_ipv4_dnat_t *session_rewrite_ipv4_dnat;
        session_rewrite_actiondata.action_id =
            SESSION_REWRITE_SESSION_REWRITE_IPV4_DNAT_ID;
        session_rewrite_ipv4_dnat =
            &session_rewrite_actiondata.action_u.session_rewrite_session_rewrite_ipv4_dnat;
        session_rewrite_ipv4_dnat->strip_outer_encap_flag = 
                spec->data.strip_encap_header;
        session_rewrite_ipv4_dnat->strip_l2_header_flag = 
            spec->data.strip_l2_header;
        session_rewrite_ipv4_dnat->strip_vlan_tag_flag = 
            spec->data.strip_vlan_tag;
        session_rewrite_ipv4_dnat->ipv4_addr_dnat =
               spec->data.nat_info.u.ipv4_addr;
        session_rewrite_ipv4_dnat->valid_flag = 1;
    } else if (spec->data.nat_info.nat_type == REWRITE_NAT_TYPE_IPV4_SDPAT) {
        session_rewrite_session_rewrite_ipv4_pat_t *session_rewrite_ipv4_pat;
        session_rewrite_actiondata.action_id =
            SESSION_REWRITE_SESSION_REWRITE_IPV4_PAT_ID;
        session_rewrite_ipv4_pat =
            &session_rewrite_actiondata.action_u.session_rewrite_session_rewrite_ipv4_pat;
        session_rewrite_ipv4_pat->strip_outer_encap_flag = 
                spec->data.strip_encap_header;
        session_rewrite_ipv4_pat->strip_l2_header_flag = 
            spec->data.strip_l2_header;
        session_rewrite_ipv4_pat->strip_vlan_tag_flag = 
            spec->data.strip_vlan_tag;
        session_rewrite_ipv4_pat->ipv4_addr_spat =
               spec->data.nat_info.u.pat.ip_saddr;
        session_rewrite_ipv4_pat->ipv4_addr_dpat =
               spec->data.nat_info.u.pat.ip_daddr;
        session_rewrite_ipv4_pat->l4_port_spat =
               spec->data.nat_info.u.pat.l4_sport;
        session_rewrite_ipv4_pat->l4_port_dpat =
               spec->data.nat_info.u.pat.l4_dport;
        session_rewrite_ipv4_pat->valid_flag = 1;
    } else if (spec->data.nat_info.nat_type == REWRITE_NAT_TYPE_IPV6_SNAT) {
        session_rewrite_session_rewrite_ipv6_snat_t *session_rewrite_ipv6_snat;
        session_rewrite_actiondata.action_id =
            SESSION_REWRITE_SESSION_REWRITE_IPV6_SNAT_ID;
        session_rewrite_ipv6_snat =
            &session_rewrite_actiondata.action_u.session_rewrite_session_rewrite_ipv6_snat;
        session_rewrite_ipv6_snat->strip_outer_encap_flag = 
                spec->data.strip_encap_header;
        session_rewrite_ipv6_snat->strip_l2_header_flag = 
            spec->data.strip_l2_header;
        session_rewrite_ipv6_snat->strip_vlan_tag_flag = 
            spec->data.strip_vlan_tag;
        memcpy(session_rewrite_ipv6_snat->ipv6_addr_snat,
               spec->data.nat_info.u.ipv6_addr,
               IP6_ADDR8_LEN);
        session_rewrite_ipv6_snat->valid_flag = 1;
    } else if (spec->data.nat_info.nat_type == REWRITE_NAT_TYPE_IPV6_DNAT) {
        session_rewrite_session_rewrite_ipv6_dnat_t *session_rewrite_ipv6_dnat;
        session_rewrite_actiondata.action_id =
            SESSION_REWRITE_SESSION_REWRITE_IPV6_DNAT_ID;
        session_rewrite_ipv6_dnat =
            &session_rewrite_actiondata.action_u.session_rewrite_session_rewrite_ipv6_dnat;
        session_rewrite_ipv6_dnat->strip_outer_encap_flag = 
                spec->data.strip_encap_header;
        session_rewrite_ipv6_dnat->strip_l2_header_flag = 
            spec->data.strip_l2_header;
        session_rewrite_ipv6_dnat->strip_vlan_tag_flag = 
            spec->data.strip_vlan_tag;
        memcpy(session_rewrite_ipv6_dnat->ipv6_addr_dnat,
               spec->data.nat_info.u.ipv6_addr,
               IP6_ADDR8_LEN);
        session_rewrite_ipv6_dnat->valid_flag = 1;
    }
    p4pd_ret = p4pd_global_entry_write(P4TBL_ID_SESSION_REWRITE,
                                       session_rewrite_id, NULL, NULL,
                                       &session_rewrite_actiondata);
    if (p4pd_ret != P4PD_SUCCESS) {
        PDS_TRACE_ERR("Failed to write session rewrite table at index %u",
                       session_rewrite_id);
        return SDK_RET_HW_PROGRAM_ERR;
    }

    // Session rewrite encap programming
    // TODO: Check if no encap case is valid
    if (spec->data.encap_type == ENCAP_TYPE_NONE) {
        return SDK_RET_OK;
    } else if (spec->data.encap_type == ENCAP_TYPE_L2) {
        session_rewrite_encap_session_rewrite_encap_l2_t *session_encap_l2 =
            &session_encap_actiondata.action_u.session_rewrite_encap_session_rewrite_encap_l2;
        session_encap_actiondata.action_id =
            SESSION_REWRITE_ENCAP_SESSION_REWRITE_ENCAP_L2_ID;
        session_encap_l2->add_vlan_tag_flag =
            spec->data.u.l2_encap.insert_vlan_tag;
        session_encap_l2->vlan =
            spec->data.u.l2_encap.vlan_id;
        memcpy(session_encap_l2->dmac,
               spec->data.u.l2_encap.dmac,
               ETH_ADDR_LEN);
        memcpy(session_encap_l2->smac,
               spec->data.u.l2_encap.smac,
               ETH_ADDR_LEN);
        session_encap_l2->valid_flag = 1;
    } else if (spec->data.encap_type == ENCAP_TYPE_MPLSOUDP) {
        session_rewrite_encap_session_rewrite_encap_mplsoudp_t *session_encap_mplsoudp =
            &session_encap_actiondata.action_u.session_rewrite_encap_session_rewrite_encap_mplsoudp;
        session_encap_actiondata.action_id =
            SESSION_REWRITE_ENCAP_SESSION_REWRITE_ENCAP_MPLSOUDP_ID;
        session_encap_mplsoudp->add_vlan_tag_flag =
            spec->data.u.mplsoudp_encap.l2_encap.insert_vlan_tag;
        session_encap_mplsoudp->vlan =
            spec->data.u.mplsoudp_encap.l2_encap.vlan_id;
        memcpy(session_encap_mplsoudp->dmac,
               spec->data.u.mplsoudp_encap.l2_encap.dmac,
               ETH_ADDR_LEN);
        memcpy(session_encap_mplsoudp->smac,
               spec->data.u.mplsoudp_encap.l2_encap.smac,
               ETH_ADDR_LEN);
        session_encap_mplsoudp->ipv4_sa =
            spec->data.u.mplsoudp_encap.ip_encap.ip_saddr;
        session_encap_mplsoudp->ipv4_da = spec->data.u.mplsoudp_encap.ip_encap.ip_daddr;
        session_encap_mplsoudp->udp_sport = spec->data.u.mplsoudp_encap.udp_encap.udp_sport;
        session_encap_mplsoudp->udp_dport = spec->data.u.mplsoudp_encap.udp_encap.udp_dport;
        session_encap_mplsoudp->mpls_label1 = spec->data.u.mplsoudp_encap.mpls1_label;
        session_encap_mplsoudp->mpls_label2 = spec->data.u.mplsoudp_encap.mpls2_label;
        session_encap_mplsoudp->mpls_label3 = spec->data.u.mplsoudp_encap.mpls3_label;
        session_encap_mplsoudp->valid_flag = 1;
    } else if (spec->data.encap_type == ENCAP_TYPE_GENEVE) {
        session_rewrite_encap_session_rewrite_encap_geneve_t *session_encap_geneve =
            &session_encap_actiondata.action_u.session_rewrite_encap_session_rewrite_encap_geneve;
        session_encap_actiondata.action_id =
            SESSION_REWRITE_ENCAP_SESSION_REWRITE_ENCAP_GENEVE_ID;
        session_encap_geneve->add_vlan_tag_flag =
            spec->data.u.mplsogre_encap.l2_encap.insert_vlan_tag;
        session_encap_geneve->vlan =
            spec->data.u.mplsogre_encap.l2_encap.vlan_id;
        memcpy(session_encap_geneve->dmac,
               spec->data.u.geneve_encap.l2_encap.dmac,
               ETH_ADDR_LEN);
        memcpy(session_encap_geneve->smac,
               spec->data.u.geneve_encap.l2_encap.smac,
               ETH_ADDR_LEN);
        session_encap_geneve->ipv4_sa =
            spec->data.u.geneve_encap.ip_encap.ip_saddr;
        session_encap_geneve->ipv4_da =
            spec->data.u.geneve_encap.ip_encap.ip_daddr;
        session_encap_geneve->udp_sport =
            spec->data.u.geneve_encap.udp_encap.udp_sport;
        session_encap_geneve->udp_dport =
            spec->data.u.geneve_encap.udp_encap.udp_dport;
        session_encap_geneve->vni =
            spec->data.u.geneve_encap.vni;
        session_encap_geneve->source_slot_id =
            spec->data.u.geneve_encap.source_slot_id;
        session_encap_geneve->destination_slot_id =
            spec->data.u.geneve_encap.destination_slot_id;
        session_encap_geneve->sg_id1 =
            spec->data.u.geneve_encap.sg_id1;
        session_encap_geneve->sg_id2 =
            spec->data.u.geneve_encap.sg_id2;
        session_encap_geneve->sg_id3 =
            spec->data.u.geneve_encap.sg_id3;
        session_encap_geneve->sg_id4 =
            spec->data.u.geneve_encap.sg_id4;
        session_encap_geneve->sg_id5 =
            spec->data.u.geneve_encap.sg_id5;
        session_encap_geneve->sg_id6 =
            spec->data.u.geneve_encap.sg_id6;
        session_encap_geneve->originator_physical_ip =
            spec->data.u.geneve_encap.originator_physical_ip;
        session_encap_geneve->valid_flag = 1;
    }
    p4pd_ret = p4pd_global_entry_write(P4TBL_ID_SESSION_REWRITE_ENCAP,
                                       session_rewrite_id, NULL, NULL,
                                       &session_encap_actiondata);
    if (p4pd_ret != P4PD_SUCCESS) {
        PDS_TRACE_ERR("Failed to write session rewrite encap table at index %u",
                       session_rewrite_id);
        return SDK_RET_HW_PROGRAM_ERR;
    }
    return SDK_RET_OK;
}

sdk_ret_t
pds_flow_session_rewrite_read (pds_flow_session_rewrite_key_t *key,
                               pds_flow_session_rewrite_info_t *info)
{
    p4pd_error_t p4pd_ret;
    uint32_t session_rewrite_id;
    session_rewrite_actiondata_t session_rewrite_actiondata = { 0 };
    session_rewrite_encap_actiondata_t session_encap_actiondata = { 0 };

    if (!key || !info) {
        PDS_TRACE_ERR("key or info is null");
        return SDK_RET_INVALID_ARG;
    }
    session_rewrite_id = key->session_rewrite_id;
    if ((session_rewrite_id == 0) ||
        (session_rewrite_id >= PDS_FLOW_SESSION_REWRITE_ID_MAX)) {
        PDS_TRACE_ERR("session id %u is invalid", session_rewrite_id);
        return SDK_RET_INVALID_ARG;
    }

    // Read rewrite NAT data
    if (p4pd_global_entry_read(P4TBL_ID_SESSION_REWRITE, session_rewrite_id,
                               NULL, NULL, &session_rewrite_actiondata) !=
                               P4PD_SUCCESS) {
        PDS_TRACE_ERR("Failed to read session rewrite table at index %u",
                      session_rewrite_id);
        return SDK_RET_HW_READ_ERR;
    }

    if (session_rewrite_actiondata.action_id ==
        SESSION_REWRITE_SESSION_REWRITE_ID) {
        session_rewrite_session_rewrite_t *session_rewrite;
        session_rewrite =
            &session_rewrite_actiondata.action_u.session_rewrite_session_rewrite;
        if (!session_rewrite->valid_flag) {
            PDS_TRACE_ERR("Invalid entry in session rewrite table at index %u",
                          session_rewrite_id);
            return SDK_RET_ERR;
        }
        info->spec.data.strip_encap_header =
            session_rewrite->strip_outer_encap_flag;
        info->spec.data.strip_l2_header =
            session_rewrite->strip_l2_header_flag;
        info->spec.data.strip_vlan_tag =
            session_rewrite->strip_vlan_tag_flag;
    } else if (session_rewrite_actiondata.action_id ==
               SESSION_REWRITE_SESSION_REWRITE_IPV4_SNAT_ID) {
        session_rewrite_session_rewrite_ipv4_snat_t *session_rewrite_ipv4_snat;
        session_rewrite_ipv4_snat =
            &session_rewrite_actiondata.action_u.session_rewrite_session_rewrite_ipv4_snat;
        if (!session_rewrite_ipv4_snat->valid_flag) {
            PDS_TRACE_ERR("Invalid entry in session rewrite table at index %u",
                          session_rewrite_id);
            return SDK_RET_ERR;
        }
        info->spec.data.strip_encap_header =
            session_rewrite_ipv4_snat->strip_outer_encap_flag;
        info->spec.data.strip_l2_header =
            session_rewrite_ipv4_snat->strip_l2_header_flag;
        info->spec.data.strip_vlan_tag =
            session_rewrite_ipv4_snat->strip_vlan_tag_flag;
        info->spec.data.nat_info.u.ipv4_addr =
            session_rewrite_ipv4_snat->ipv4_addr_snat;
    } else if (session_rewrite_actiondata.action_id ==
               SESSION_REWRITE_SESSION_REWRITE_IPV4_DNAT_ID) {
        session_rewrite_session_rewrite_ipv4_dnat_t *session_rewrite_ipv4_dnat;
        session_rewrite_ipv4_dnat =
            &session_rewrite_actiondata.action_u.session_rewrite_session_rewrite_ipv4_dnat;
        if (!session_rewrite_ipv4_dnat->valid_flag) {
            PDS_TRACE_ERR("Invalid entry in session rewrite table at index %u",
                          session_rewrite_id);
            return SDK_RET_ERR;
        }
        info->spec.data.strip_encap_header =
            session_rewrite_ipv4_dnat->strip_outer_encap_flag;
        info->spec.data.strip_l2_header =
            session_rewrite_ipv4_dnat->strip_l2_header_flag;
        info->spec.data.strip_vlan_tag =
            session_rewrite_ipv4_dnat->strip_vlan_tag_flag;
        info->spec.data.nat_info.u.ipv4_addr =
            session_rewrite_ipv4_dnat->ipv4_addr_dnat;
    } else if (session_rewrite_actiondata.action_id ==
               SESSION_REWRITE_SESSION_REWRITE_IPV4_PAT_ID) {
        session_rewrite_session_rewrite_ipv4_pat_t *session_rewrite_ipv4_pat;
        session_rewrite_ipv4_pat =
            &session_rewrite_actiondata.action_u.session_rewrite_session_rewrite_ipv4_pat;
        if (!session_rewrite_ipv4_pat->valid_flag) {
            PDS_TRACE_ERR("Invalid entry in session rewrite table at index %u",
                          session_rewrite_id);
            return SDK_RET_ERR;
        }
        info->spec.data.strip_encap_header =
            session_rewrite_ipv4_pat->strip_outer_encap_flag;
        info->spec.data.strip_l2_header =
            session_rewrite_ipv4_pat->strip_l2_header_flag;
        info->spec.data.strip_vlan_tag =
            session_rewrite_ipv4_pat->strip_vlan_tag_flag;
        info->spec.data.nat_info.u.pat.ip_saddr =
            session_rewrite_ipv4_pat->ipv4_addr_spat;
        info->spec.data.nat_info.u.pat.ip_daddr =
            session_rewrite_ipv4_pat->ipv4_addr_dpat;
        info->spec.data.nat_info.u.pat.l4_sport =
            session_rewrite_ipv4_pat->l4_port_spat;
        info->spec.data.nat_info.u.pat.l4_dport =
            session_rewrite_ipv4_pat->l4_port_dpat;
    } else if (session_rewrite_actiondata.action_id ==
               SESSION_REWRITE_SESSION_REWRITE_IPV6_SNAT_ID) {
        session_rewrite_session_rewrite_ipv6_snat_t *session_rewrite_ipv6_snat;
        session_rewrite_ipv6_snat =
            &session_rewrite_actiondata.action_u.session_rewrite_session_rewrite_ipv6_snat;
        if (!session_rewrite_ipv6_snat->valid_flag) {
            PDS_TRACE_ERR("Invalid entry in session rewrite table at index %u",
                          session_rewrite_id);
            return SDK_RET_ERR;
        }
        info->spec.data.strip_encap_header =
            session_rewrite_ipv6_snat->strip_outer_encap_flag;
        info->spec.data.strip_l2_header =
            session_rewrite_ipv6_snat->strip_l2_header_flag;
        info->spec.data.strip_vlan_tag =
            session_rewrite_ipv6_snat->strip_vlan_tag_flag;
        memcpy(info->spec.data.nat_info.u.ipv6_addr,
               session_rewrite_ipv6_snat->ipv6_addr_snat,
               IP6_ADDR8_LEN);
    } else if (session_rewrite_actiondata.action_id ==
               SESSION_REWRITE_SESSION_REWRITE_IPV6_DNAT_ID) {
        session_rewrite_session_rewrite_ipv6_dnat_t *session_rewrite_ipv6_dnat;
        session_rewrite_ipv6_dnat =
            &session_rewrite_actiondata.action_u.session_rewrite_session_rewrite_ipv6_dnat;
        if (!session_rewrite_ipv6_dnat->valid_flag) {
            PDS_TRACE_ERR("Invalid entry in session rewrite table at index %u",
                          session_rewrite_id);
            return SDK_RET_ERR;
        }
        info->spec.data.strip_encap_header =
            session_rewrite_ipv6_dnat->strip_outer_encap_flag;
        info->spec.data.strip_l2_header =
            session_rewrite_ipv6_dnat->strip_l2_header_flag;
        info->spec.data.strip_vlan_tag =
            session_rewrite_ipv6_dnat->strip_vlan_tag_flag;
        memcpy(info->spec.data.nat_info.u.ipv6_addr,
               session_rewrite_ipv6_dnat->ipv6_addr_dnat,
               IP6_ADDR8_LEN);
    } else {
        PDS_TRACE_ERR("Invalid action/entry in session rewrite table"
                      " at index %u", session_rewrite_id);
        return SDK_RET_ERR;
    }

    // Populate encap data
    p4pd_ret = p4pd_global_entry_read(P4TBL_ID_SESSION_REWRITE_ENCAP,
                                       session_rewrite_id, NULL, NULL,
                                       &session_encap_actiondata);
    if (p4pd_ret != P4PD_SUCCESS) {
        PDS_TRACE_ERR("Failed to read session rewrite encap table at index %u",
                       session_rewrite_id);
        return SDK_RET_HW_PROGRAM_ERR;
    }
    if (session_encap_actiondata.action_id ==
        SESSION_REWRITE_ENCAP_SESSION_REWRITE_ENCAP_L2_ID) {
        session_rewrite_encap_session_rewrite_encap_l2_t *session_encap_l2 =
            &session_encap_actiondata.action_u.session_rewrite_encap_session_rewrite_encap_l2;
        if (!session_encap_l2->valid_flag) {
            PDS_TRACE_ERR("Invalid entry in session encap table at index %u",
                          session_rewrite_id);
            return SDK_RET_ERR;
        }
        info->spec.data.u.l2_encap.insert_vlan_tag =
            session_encap_l2->add_vlan_tag_flag;
        info->spec.data.u.l2_encap.vlan_id =
            session_encap_l2->vlan;
        memcpy(info->spec.data.u.l2_encap.dmac,
               session_encap_l2->dmac,
               ETH_ADDR_LEN);
        memcpy(info->spec.data.u.l2_encap.smac,
               session_encap_l2->smac,
               ETH_ADDR_LEN);
    } else if (session_encap_actiondata.action_id ==
        SESSION_REWRITE_ENCAP_SESSION_REWRITE_ENCAP_MPLSOUDP_ID) {
        session_rewrite_encap_session_rewrite_encap_mplsoudp_t *session_encap_mplsoudp =
            &session_encap_actiondata.action_u.session_rewrite_encap_session_rewrite_encap_mplsoudp;
        if (!session_encap_mplsoudp->valid_flag) {
            PDS_TRACE_ERR("Invalid entry in session encap table at index %u",
                          session_rewrite_id);
            return SDK_RET_ERR;
        }
        info->spec.data.u.mplsoudp_encap.l2_encap.insert_vlan_tag =
            session_encap_mplsoudp->add_vlan_tag_flag;
        info->spec.data.u.mplsoudp_encap.l2_encap.vlan_id =
            session_encap_mplsoudp->vlan;
        memcpy(info->spec.data.u.mplsoudp_encap.l2_encap.dmac,
               session_encap_mplsoudp->dmac,
               ETH_ADDR_LEN);
        memcpy(info->spec.data.u.mplsoudp_encap.l2_encap.smac,
               session_encap_mplsoudp->smac,
               ETH_ADDR_LEN);
        info->spec.data.u.mplsoudp_encap.ip_encap.ip_saddr =
            session_encap_mplsoudp->ipv4_sa;
        info->spec.data.u.mplsoudp_encap.ip_encap.ip_daddr =
            session_encap_mplsoudp->ipv4_da;
	info->spec.data.u.mplsoudp_encap.udp_encap.udp_sport =
            session_encap_mplsoudp->udp_sport;
	info->spec.data.u.mplsoudp_encap.udp_encap.udp_dport =
            session_encap_mplsoudp->udp_dport;
        info->spec.data.u.mplsoudp_encap.mpls1_label =
            session_encap_mplsoudp->mpls_label1;
        info->spec.data.u.mplsoudp_encap.mpls2_label =
            session_encap_mplsoudp->mpls_label2;
        info->spec.data.u.mplsoudp_encap.mpls3_label =
            session_encap_mplsoudp->mpls_label3;
    } else if (session_encap_actiondata.action_id ==
        SESSION_REWRITE_ENCAP_SESSION_REWRITE_ENCAP_GENEVE_ID) {
        session_rewrite_encap_session_rewrite_encap_geneve_t *session_encap_geneve =
            &session_encap_actiondata.action_u.session_rewrite_encap_session_rewrite_encap_geneve;
        if (!session_encap_geneve->valid_flag) {
            PDS_TRACE_ERR("Invalid entry in session encap table at index %u",
                          session_rewrite_id);
            return SDK_RET_ERR;
        }
        info->spec.data.u.geneve_encap.l2_encap.insert_vlan_tag =
            session_encap_geneve->add_vlan_tag_flag;
        info->spec.data.u.geneve_encap.l2_encap.vlan_id =
            session_encap_geneve->vlan;
        memcpy(info->spec.data.u.geneve_encap.l2_encap.dmac,
               session_encap_geneve->dmac,
               ETH_ADDR_LEN);
        memcpy(info->spec.data.u.geneve_encap.l2_encap.smac,
               session_encap_geneve->smac,
               ETH_ADDR_LEN);
        info->spec.data.u.geneve_encap.ip_encap.ip_saddr =
            session_encap_geneve->ipv4_sa;
        info->spec.data.u.geneve_encap.ip_encap.ip_daddr =
            session_encap_geneve->ipv4_da;
	info->spec.data.u.geneve_encap.udp_encap.udp_sport =
            session_encap_geneve->udp_sport;
	info->spec.data.u.geneve_encap.udp_encap.udp_dport =
            session_encap_geneve->udp_dport;
        info->spec.data.u.geneve_encap.vni =
            session_encap_geneve->vni;
        info->spec.data.u.geneve_encap.source_slot_id =
            session_encap_geneve->source_slot_id;
        info->spec.data.u.geneve_encap.destination_slot_id =
            session_encap_geneve->destination_slot_id;
        info->spec.data.u.geneve_encap.sg_id1 =
            session_encap_geneve->sg_id1;
        info->spec.data.u.geneve_encap.sg_id2 =
            session_encap_geneve->sg_id2;
        info->spec.data.u.geneve_encap.sg_id3 =
            session_encap_geneve->sg_id3;
        info->spec.data.u.geneve_encap.sg_id4 =
            session_encap_geneve->sg_id4;
        info->spec.data.u.geneve_encap.sg_id5 =
            session_encap_geneve->sg_id5;
        info->spec.data.u.geneve_encap.sg_id6 =
            session_encap_geneve->sg_id6;
        info->spec.data.u.geneve_encap.originator_physical_ip =
            session_encap_geneve->originator_physical_ip;
    } else {
        PDS_TRACE_ERR("Invalid action/entry in session encap table"
                      " at index %u", session_rewrite_id);
        return SDK_RET_ERR;
    }
    return SDK_RET_OK;
}

sdk_ret_t
pds_flow_session_rewrite_update (pds_flow_session_rewrite_spec_t *spec)
{
    return pds_flow_session_rewrite_create(spec);
}

sdk_ret_t
pds_flow_session_rewrite_delete (pds_flow_session_rewrite_key_t *key)
{
    p4pd_error_t p4pd_ret;
    uint32_t session_rewrite_id;
    session_rewrite_actiondata_t session_rewrite_actiondata = { 0 };
    session_rewrite_encap_actiondata_t session_encap_actiondata = { 0 };

    if (!key) {
        PDS_TRACE_ERR("key is null");
        return SDK_RET_INVALID_ARG;
    }
    session_rewrite_id = key->session_rewrite_id;
    if ((session_rewrite_id == 0) ||
        (session_rewrite_id >= PDS_FLOW_SESSION_REWRITE_ID_MAX)) {
        PDS_TRACE_ERR("session rewrite id %u is invalid",
                      session_rewrite_id);
        return SDK_RET_INVALID_ARG;
    }

    p4pd_ret = p4pd_global_entry_write(P4TBL_ID_SESSION_REWRITE,
                                       session_rewrite_id, NULL, NULL,
                                       &session_rewrite_actiondata);
    if (p4pd_ret != P4PD_SUCCESS) {
        PDS_TRACE_ERR("Failed to delete session rewrite table at index %u",
                      session_rewrite_id);
        return SDK_RET_HW_PROGRAM_ERR;
    }
    p4pd_ret = p4pd_global_entry_write(P4TBL_ID_SESSION_REWRITE_ENCAP,
                                       session_rewrite_id, NULL, NULL,
                                       &session_encap_actiondata);
    if (p4pd_ret != P4PD_SUCCESS) {
        PDS_TRACE_ERR("Failed to delete session encap table at index %u",
                      session_rewrite_id);
        return SDK_RET_HW_PROGRAM_ERR;
    }
    return SDK_RET_OK;
}

}
