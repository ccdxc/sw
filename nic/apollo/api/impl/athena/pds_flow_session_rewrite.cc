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
#include "gen/p4gen/p4/include/ftl.h"

using namespace sdk;

extern "C" {

sdk_ret_t
pds_flow_session_rewrite_create (pds_flow_session_rewrite_spec_t *spec)
{
    p4pd_error_t p4pd_ret = P4PD_SUCCESS;
    uint32_t session_rewrite_id;
    uint8_t   action_pc = 0xff;

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

        session_rewrite_entry_t session_rewrite;

        session_rewrite.clear();

        action_pc = sdk::asic::pd::asicpd_get_action_pc(
                               P4TBL_ID_SESSION_REWRITE, 
                               SESSION_REWRITE_SESSION_REWRITE_ID);
        session_rewrite.set_actionid( action_pc);
            //SESSION_REWRITE_SESSION_REWRITE_ID);
        session_rewrite.set_strip_outer_encap_flag( 
                spec->data.strip_encap_header);
        session_rewrite.set_strip_l2_header_flag( 
            spec->data.strip_l2_header);
        session_rewrite.set_strip_vlan_tag_flag(
            spec->data.strip_vlan_tag);
        session_rewrite.set_valid_flag(TRUE);

        p4pd_ret = session_rewrite.write(session_rewrite_id);

        

    } else if (spec->data.nat_info.nat_type == REWRITE_NAT_TYPE_IPV4_SNAT) {

        session_rewrite_ipv4_snat_entry_t session_rewrite_ipv4_snat;

        session_rewrite_ipv4_snat.clear();

        //session_rewrite_ipv4_snat.set_actionid(
         //   SESSION_REWRITE_SESSION_REWRITE_IPV4_SNAT_ID);
        action_pc = sdk::asic::pd::asicpd_get_action_pc(
                        P4TBL_ID_SESSION_REWRITE, 
                        SESSION_REWRITE_SESSION_REWRITE_IPV4_SNAT_ID);
        session_rewrite_ipv4_snat.set_actionid( action_pc);
        session_rewrite_ipv4_snat.set_ipv4_addr_snat(
            spec->data.nat_info.u.ipv4_addr);
        session_rewrite_ipv4_snat.set_strip_outer_encap_flag(
            spec->data.strip_encap_header);
        session_rewrite_ipv4_snat.set_strip_l2_header_flag(
            spec->data.strip_l2_header);
        session_rewrite_ipv4_snat.set_strip_vlan_tag_flag(
            spec->data.strip_vlan_tag);
        session_rewrite_ipv4_snat.set_valid_flag(TRUE);

        p4pd_ret = session_rewrite_ipv4_snat.write(session_rewrite_id);
        
    } else if (spec->data.nat_info.nat_type == REWRITE_NAT_TYPE_IPV4_DNAT) {
        session_rewrite_ipv4_dnat_entry_t session_rewrite_ipv4_dnat;

        session_rewrite_ipv4_dnat.clear();

        action_pc = sdk::asic::pd::asicpd_get_action_pc(
                        P4TBL_ID_SESSION_REWRITE, 
                        SESSION_REWRITE_SESSION_REWRITE_IPV4_DNAT_ID);
        session_rewrite_ipv4_dnat.set_actionid( action_pc);
        //session_rewrite_ipv4_dnat.set_actionid(
        //    SESSION_REWRITE_SESSION_REWRITE_IPV4_DNAT_ID);
        session_rewrite_ipv4_dnat.set_ipv4_addr_dnat(
            spec->data.nat_info.u.ipv4_addr);
        session_rewrite_ipv4_dnat.set_strip_outer_encap_flag(
            spec->data.strip_encap_header);
        session_rewrite_ipv4_dnat.set_strip_l2_header_flag(
            spec->data.strip_l2_header);
        session_rewrite_ipv4_dnat.set_strip_vlan_tag_flag(
            spec->data.strip_vlan_tag);
        session_rewrite_ipv4_dnat.set_valid_flag(TRUE);
    
        p4pd_ret = session_rewrite_ipv4_dnat.write(session_rewrite_id);

    } else if (spec->data.nat_info.nat_type == REWRITE_NAT_TYPE_IPV4_SDPAT) {

        session_rewrite_ipv4_pat_entry_t session_rewrite_ipv4_pat;

        session_rewrite_ipv4_pat.clear();

        action_pc = sdk::asic::pd::asicpd_get_action_pc(
                        P4TBL_ID_SESSION_REWRITE, 
                        SESSION_REWRITE_SESSION_REWRITE_IPV4_PAT_ID);
        session_rewrite_ipv4_pat.set_actionid(action_pc);
                     //SESSION_REWRITE_SESSION_REWRITE_IPV4_PAT_ID);
        session_rewrite_ipv4_pat.set_strip_outer_encap_flag(
                spec->data.strip_encap_header);
        session_rewrite_ipv4_pat.set_strip_l2_header_flag(
            spec->data.strip_l2_header);
        session_rewrite_ipv4_pat.set_strip_vlan_tag_flag(
            spec->data.strip_vlan_tag);
        session_rewrite_ipv4_pat.set_ipv4_addr_spat(
               spec->data.nat_info.u.pat.ip_saddr);
        session_rewrite_ipv4_pat.set_ipv4_addr_dpat(
               spec->data.nat_info.u.pat.ip_daddr);
        session_rewrite_ipv4_pat.set_l4_port_spat(
               spec->data.nat_info.u.pat.l4_sport);
        session_rewrite_ipv4_pat.set_l4_port_dpat(
               spec->data.nat_info.u.pat.l4_dport);
        session_rewrite_ipv4_pat.set_valid_flag(TRUE);

        p4pd_ret = session_rewrite_ipv4_pat.write(session_rewrite_id);

    } else if (spec->data.nat_info.nat_type == REWRITE_NAT_TYPE_IPV6_SNAT) {
        session_rewrite_ipv6_snat_entry_t session_rewrite_ipv6_snat;

        session_rewrite_ipv6_snat.clear();

        action_pc = sdk::asic::pd::asicpd_get_action_pc(
                        P4TBL_ID_SESSION_REWRITE, 
                        SESSION_REWRITE_SESSION_REWRITE_IPV6_SNAT_ID);
        session_rewrite_ipv6_snat.set_actionid(action_pc);
        //session_rewrite_ipv6_snat.set_actionid(
        //    SESSION_REWRITE_SESSION_REWRITE_IPV6_SNAT_ID);
        session_rewrite_ipv6_snat.set_strip_outer_encap_flag( 
                spec->data.strip_encap_header);
        session_rewrite_ipv6_snat.set_strip_l2_header_flag( 
            spec->data.strip_l2_header);
        session_rewrite_ipv6_snat.set_strip_vlan_tag_flag( 
            spec->data.strip_vlan_tag);
        session_rewrite_ipv6_snat.set_ipv6_addr_snat(
               spec->data.nat_info.u.ipv6_addr);
        session_rewrite_ipv6_snat.set_valid_flag(TRUE);

        p4pd_ret = session_rewrite_ipv6_snat.write(session_rewrite_id);

    } else if (spec->data.nat_info.nat_type == REWRITE_NAT_TYPE_IPV6_DNAT) {
        session_rewrite_ipv6_dnat_entry_t session_rewrite_ipv6_dnat;

        session_rewrite_ipv6_dnat.clear();

        action_pc = sdk::asic::pd::asicpd_get_action_pc(
                        P4TBL_ID_SESSION_REWRITE, 
                        SESSION_REWRITE_SESSION_REWRITE_IPV6_DNAT_ID);
        session_rewrite_ipv6_dnat.set_actionid(action_pc);
        //session_rewrite_ipv6_dnat.set_actionid(
        //    SESSION_REWRITE_SESSION_REWRITE_IPV6_DNAT_ID);
        session_rewrite_ipv6_dnat.set_strip_outer_encap_flag( 
                spec->data.strip_encap_header);
        session_rewrite_ipv6_dnat.set_strip_l2_header_flag( 
            spec->data.strip_l2_header);
        session_rewrite_ipv6_dnat.set_strip_vlan_tag_flag( 
            spec->data.strip_vlan_tag);
        session_rewrite_ipv6_dnat.set_ipv6_addr_dnat(
               spec->data.nat_info.u.ipv6_addr);
        session_rewrite_ipv6_dnat.set_valid_flag(TRUE);

        p4pd_ret = session_rewrite_ipv6_dnat.write(session_rewrite_id);
    }
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

        session_rewrite_encap_l2_entry_t session_encap_l2;
        uint64_t                         mac = 0;

        session_encap_l2.clear();

        session_encap_l2.set_actionid(
            sdk::asic::pd::asicpd_get_action_pc(
                P4TBL_ID_SESSION_REWRITE_ENCAP, 
                SESSION_REWRITE_ENCAP_SESSION_REWRITE_ENCAP_L2_ID));
        session_encap_l2.set_add_vlan_tag_flag(
            spec->data.u.l2_encap.insert_vlan_tag);
        session_encap_l2.set_vlan(
            spec->data.u.l2_encap.vlan_id);
        memcpy(&mac,
               spec->data.u.l2_encap.dmac,
               ETH_ADDR_LEN);
        session_encap_l2.set_dmac(mac);
        mac = 0;
        memcpy(&mac,
               spec->data.u.l2_encap.smac,
               ETH_ADDR_LEN);
        session_encap_l2.set_smac(mac);
        session_encap_l2.set_valid_flag(TRUE);

        p4pd_ret = session_encap_l2.write(session_rewrite_id);
    } else if (spec->data.encap_type == ENCAP_TYPE_MPLSOUDP) {

        session_rewrite_encap_mplsoudp_entry_t session_encap_mplsoudp;
        uint64_t                               mac = 0;

        session_encap_mplsoudp.clear();

        session_encap_mplsoudp.set_actionid(
            sdk::asic::pd::asicpd_get_action_pc(
                P4TBL_ID_SESSION_REWRITE_ENCAP, 
                SESSION_REWRITE_ENCAP_SESSION_REWRITE_ENCAP_MPLSOUDP_ID));
        session_encap_mplsoudp.set_add_vlan_tag_flag(
            spec->data.u.mplsoudp_encap.l2_encap.insert_vlan_tag);
        session_encap_mplsoudp.set_vlan(
            spec->data.u.mplsoudp_encap.l2_encap.vlan_id);
        memcpy(&mac,
               spec->data.u.mplsoudp_encap.l2_encap.dmac,
               ETH_ADDR_LEN);
        session_encap_mplsoudp.set_dmac(mac);
        mac = 0;
        memcpy(&mac,
               spec->data.u.mplsoudp_encap.l2_encap.smac,
               ETH_ADDR_LEN);
        session_encap_mplsoudp.set_smac(mac);
        session_encap_mplsoudp.set_ipv4_sa(
            spec->data.u.mplsoudp_encap.ip_encap.ip_saddr);
        session_encap_mplsoudp.set_ipv4_da(
                spec->data.u.mplsoudp_encap.ip_encap.ip_daddr);
        session_encap_mplsoudp.set_udp_sport(
                spec->data.u.mplsoudp_encap.udp_encap.udp_sport);
        session_encap_mplsoudp.set_udp_dport(
                spec->data.u.mplsoudp_encap.udp_encap.udp_dport);
        session_encap_mplsoudp.set_mpls_label1(
                spec->data.u.mplsoudp_encap.mpls1_label);
        session_encap_mplsoudp.set_mpls_label2(
                spec->data.u.mplsoudp_encap.mpls2_label);
        session_encap_mplsoudp.set_mpls_label3(
                spec->data.u.mplsoudp_encap.mpls3_label);
        session_encap_mplsoudp.set_valid_flag(TRUE);

        p4pd_ret =  session_encap_mplsoudp.write(session_rewrite_id);

    } else if (spec->data.encap_type == ENCAP_TYPE_GENEVE) {
        session_rewrite_encap_geneve_entry_t session_encap_geneve;
        uint64_t   mac = 0;

        session_encap_geneve.clear();

        session_encap_geneve.set_actionid(
            sdk::asic::pd::asicpd_get_action_pc(
                P4TBL_ID_SESSION_REWRITE_ENCAP, 
                SESSION_REWRITE_ENCAP_SESSION_REWRITE_ENCAP_GENEVE_ID));
        session_encap_geneve.set_add_vlan_tag_flag(
            spec->data.u.mplsogre_encap.l2_encap.insert_vlan_tag);
        session_encap_geneve.set_vlan(
            spec->data.u.mplsogre_encap.l2_encap.vlan_id);
        memcpy(&mac,
               spec->data.u.geneve_encap.l2_encap.dmac,
               ETH_ADDR_LEN);
        session_encap_geneve.set_dmac(mac);
        mac = 0;
        memcpy(&mac,
               spec->data.u.geneve_encap.l2_encap.smac,
               ETH_ADDR_LEN);
        session_encap_geneve.set_smac(mac);
        session_encap_geneve.set_ipv4_sa(
            spec->data.u.geneve_encap.ip_encap.ip_saddr);
        session_encap_geneve.set_ipv4_da(
            spec->data.u.geneve_encap.ip_encap.ip_daddr);
        session_encap_geneve.set_udp_sport(
            spec->data.u.geneve_encap.udp_encap.udp_sport);
        session_encap_geneve.set_udp_dport(
            spec->data.u.geneve_encap.udp_encap.udp_dport);
        session_encap_geneve.set_vni(
            spec->data.u.geneve_encap.vni);
        session_encap_geneve.set_source_slot_id(
            spec->data.u.geneve_encap.source_slot_id);
        session_encap_geneve.set_destination_slot_id(
            spec->data.u.geneve_encap.destination_slot_id);
        session_encap_geneve.set_sg_id1(
            spec->data.u.geneve_encap.sg_id1);
        session_encap_geneve.set_sg_id2(
            spec->data.u.geneve_encap.sg_id2);
        session_encap_geneve.set_sg_id3(
            spec->data.u.geneve_encap.sg_id3);
        session_encap_geneve.set_sg_id4(
            spec->data.u.geneve_encap.sg_id4);
        session_encap_geneve.set_sg_id5(
            spec->data.u.geneve_encap.sg_id5);
        session_encap_geneve.set_sg_id6(
            spec->data.u.geneve_encap.sg_id6);
        session_encap_geneve.set_originator_physical_ip(
            spec->data.u.geneve_encap.originator_physical_ip);
        session_encap_geneve.set_valid_flag(TRUE);

        p4pd_ret = session_encap_geneve.write(session_rewrite_id);
    }
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
    p4pd_error_t            p4pd_ret = SDK_RET_OK;
    uint32_t                session_rewrite_id = 0;
    session_rewrite_entry_t session_rewrite = {0};
    uint8_t                 action_id = SESSION_REWRITE_SESSION_REWRITE_ID;
    uint8_t                 action_pc = 0xff;


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

    session_rewrite.clear();
    if (session_rewrite.read(session_rewrite_id) != P4PD_SUCCESS) {
        PDS_TRACE_ERR("Failed to read session rewrite table at index %u",
                      session_rewrite_id);
        return SDK_RET_HW_READ_ERR;
    }

    action_pc = session_rewrite.get_actionid();
    action_id = sdk::asic::pd::asicpd_get_action_id(P4TBL_ID_SESSION_REWRITE, 
                                                    action_pc);
    if (action_id == 
        SESSION_REWRITE_SESSION_REWRITE_ID) {
        if (!session_rewrite.get_valid_flag()) {
            PDS_TRACE_ERR("Invalid entry in session rewrite table at index %u",
                          session_rewrite_id);
            return SDK_RET_ERR;
        }
        info->spec.data.strip_encap_header =
            session_rewrite.get_strip_outer_encap_flag();
        info->spec.data.strip_l2_header =
            session_rewrite.get_strip_l2_header_flag();
        info->spec.data.strip_vlan_tag =
            session_rewrite.get_strip_vlan_tag_flag();
        info->spec.data.nat_info.nat_type = REWRITE_NAT_TYPE_NONE;
    } else if (action_id == 
               SESSION_REWRITE_SESSION_REWRITE_IPV4_SNAT_ID) {

        session_rewrite_ipv4_snat_entry_t  session_rewrite_ipv4_snat;

        session_rewrite_ipv4_snat.copy_data((void *)&session_rewrite);

        if (!session_rewrite_ipv4_snat.get_valid_flag()) {
            PDS_TRACE_ERR("Invalid entry in session rewrite table at index %u",
                          session_rewrite_id);
            return SDK_RET_ERR;
        }
        info->spec.data.strip_encap_header =
            session_rewrite_ipv4_snat.get_strip_outer_encap_flag();
        info->spec.data.strip_l2_header =
            session_rewrite_ipv4_snat.get_strip_l2_header_flag();
        info->spec.data.strip_vlan_tag =
            session_rewrite_ipv4_snat.get_strip_vlan_tag_flag();
        info->spec.data.nat_info.u.ipv4_addr =
            session_rewrite_ipv4_snat.get_ipv4_addr_snat();
        info->spec.data.nat_info.nat_type = REWRITE_NAT_TYPE_IPV4_SNAT;
    } else if (action_id ==
               SESSION_REWRITE_SESSION_REWRITE_IPV4_DNAT_ID) {

        session_rewrite_ipv4_dnat_entry_t session_rewrite_ipv4_dnat;

        session_rewrite_ipv4_dnat.clear();
        session_rewrite_ipv4_dnat.copy_data((void *)&session_rewrite);

        if (!session_rewrite_ipv4_dnat.get_valid_flag()) {
            PDS_TRACE_ERR("Invalid entry in session rewrite table at index %u",
                          session_rewrite_id);
            return SDK_RET_ERR;
        }
        info->spec.data.strip_encap_header =
            session_rewrite_ipv4_dnat.get_strip_outer_encap_flag();
        info->spec.data.strip_l2_header =
            session_rewrite_ipv4_dnat.get_strip_l2_header_flag();
        info->spec.data.strip_vlan_tag =
            session_rewrite_ipv4_dnat.get_strip_vlan_tag_flag();
        info->spec.data.nat_info.u.ipv4_addr =
            session_rewrite_ipv4_dnat.get_ipv4_addr_dnat();
        info->spec.data.nat_info.nat_type = REWRITE_NAT_TYPE_IPV4_DNAT;

    } else if (action_id == 
               SESSION_REWRITE_SESSION_REWRITE_IPV4_PAT_ID) {

        session_rewrite_ipv4_pat_entry_t session_rewrite_ipv4_pat;

        session_rewrite_ipv4_pat.clear();
        session_rewrite_ipv4_pat.copy_data((void *)&session_rewrite);

        if (!session_rewrite_ipv4_pat.get_valid_flag()) {
            PDS_TRACE_ERR("Invalid entry in session rewrite table at index %u",
                          session_rewrite_id);
            return SDK_RET_ERR;
        }
        info->spec.data.strip_encap_header =
            session_rewrite_ipv4_pat.get_strip_outer_encap_flag();
        info->spec.data.strip_l2_header =
            session_rewrite_ipv4_pat.get_strip_l2_header_flag();
        info->spec.data.strip_vlan_tag =
            session_rewrite_ipv4_pat.get_strip_vlan_tag_flag();
        info->spec.data.nat_info.u.pat.ip_saddr =
            session_rewrite_ipv4_pat.get_ipv4_addr_spat();
        info->spec.data.nat_info.u.pat.ip_daddr =
            session_rewrite_ipv4_pat.get_ipv4_addr_dpat();
        info->spec.data.nat_info.u.pat.l4_sport =
            session_rewrite_ipv4_pat.get_l4_port_spat();
        info->spec.data.nat_info.u.pat.l4_dport =
            session_rewrite_ipv4_pat.get_l4_port_dpat();
        info->spec.data.nat_info.nat_type = REWRITE_NAT_TYPE_IPV4_SDPAT;

    } else if (action_id == 
               SESSION_REWRITE_SESSION_REWRITE_IPV6_SNAT_ID) {
        session_rewrite_ipv6_snat_entry_t session_rewrite_ipv6_snat;

        session_rewrite_ipv6_snat.clear();
        session_rewrite_ipv6_snat.copy_data((void *)&session_rewrite);
      
        if (!session_rewrite_ipv6_snat.get_valid_flag()) {
            PDS_TRACE_ERR("Invalid entry in session rewrite table at index %u",
                          session_rewrite_id);
            return SDK_RET_ERR;
        }
        info->spec.data.strip_encap_header =
            session_rewrite_ipv6_snat.get_strip_outer_encap_flag();
        info->spec.data.strip_l2_header =
            session_rewrite_ipv6_snat.get_strip_l2_header_flag();
        info->spec.data.strip_vlan_tag =
            session_rewrite_ipv6_snat.get_strip_vlan_tag_flag();
        session_rewrite_ipv6_snat.get_ipv6_addr_snat(
            info->spec.data.nat_info.u.ipv6_addr);
        info->spec.data.nat_info.nat_type = REWRITE_NAT_TYPE_IPV6_SNAT;

    } else if (action_id == 
               SESSION_REWRITE_SESSION_REWRITE_IPV6_DNAT_ID) {

        session_rewrite_ipv6_dnat_entry_t session_rewrite_ipv6_dnat;

        session_rewrite_ipv6_dnat.clear();
        session_rewrite_ipv6_dnat.copy_data((void *)&session_rewrite);

        if (!session_rewrite_ipv6_dnat.get_valid_flag()) {
            PDS_TRACE_ERR("Invalid entry in session rewrite table at index %u",
                          session_rewrite_id);
            return SDK_RET_ERR;
        }
        info->spec.data.strip_encap_header =
            session_rewrite_ipv6_dnat.get_strip_outer_encap_flag();
        info->spec.data.strip_l2_header =
            session_rewrite_ipv6_dnat.get_strip_l2_header_flag();
        info->spec.data.strip_vlan_tag =
            session_rewrite_ipv6_dnat.get_strip_vlan_tag_flag();
        session_rewrite_ipv6_dnat.get_ipv6_addr_dnat(
            info->spec.data.nat_info.u.ipv6_addr);
        info->spec.data.nat_info.nat_type = REWRITE_NAT_TYPE_IPV6_DNAT;

    } else {
        PDS_TRACE_ERR("Invalid action/entry in session rewrite table"
                      " at index %u", session_rewrite_id);
        return SDK_RET_ERR;
    }


    session_rewrite_encap_l2_entry_t  session_encap_l2 = {0};
    uint64_t                          mac = 0;
    session_encap_l2.clear();

    if (session_encap_l2.read(session_rewrite_id) != P4PD_SUCCESS) {    
        PDS_TRACE_ERR("Failed to read session rewrite encap table at index %u",
                       session_rewrite_id);
        return SDK_RET_HW_PROGRAM_ERR;
    }

    action_pc = session_encap_l2.get_actionid();
    action_id = sdk::asic::pd::asicpd_get_action_id(
                             P4TBL_ID_SESSION_REWRITE_ENCAP, 
                             action_pc);

    if (action_id == 
        SESSION_REWRITE_ENCAP_SESSION_REWRITE_ENCAP_L2_ID) {
        if (!session_encap_l2.get_valid_flag()) {
            PDS_TRACE_ERR("Invalid entry in session encap table at index %u",
                          session_rewrite_id);
            return SDK_RET_ERR;
        }
        info->spec.data.u.l2_encap.insert_vlan_tag =
            session_encap_l2.get_add_vlan_tag_flag();
        info->spec.data.u.l2_encap.vlan_id =
            session_encap_l2.get_vlan();

        mac = session_encap_l2.get_dmac();
        memcpy(info->spec.data.u.l2_encap.dmac,
               &mac, 
               ETH_ADDR_LEN);
        mac = 0;
        mac = session_encap_l2.get_smac();
        memcpy(info->spec.data.u.l2_encap.smac, &mac,
               ETH_ADDR_LEN);
        info->spec.data.encap_type = ENCAP_TYPE_L2;
    } else if (action_id ==
        SESSION_REWRITE_ENCAP_SESSION_REWRITE_ENCAP_MPLSOUDP_ID) {

        session_rewrite_encap_mplsoudp_entry_t session_encap_mplsoudp ;

        session_encap_mplsoudp.clear();
        session_encap_mplsoudp.copy_data(&session_encap_l2);

        if (!session_encap_mplsoudp.get_valid_flag()) {
            PDS_TRACE_ERR("Invalid entry in session encap table at index %u",
                          session_rewrite_id);
            return SDK_RET_ERR;
        }
        info->spec.data.u.mplsoudp_encap.l2_encap.insert_vlan_tag =
            session_encap_mplsoudp.get_add_vlan_tag_flag();
        info->spec.data.u.mplsoudp_encap.l2_encap.vlan_id =
            session_encap_mplsoudp.get_vlan();

        mac = session_encap_mplsoudp.get_dmac();
        memcpy(info->spec.data.u.mplsoudp_encap.l2_encap.dmac, &mac,
               ETH_ADDR_LEN);
        mac = session_encap_mplsoudp.get_smac();
        memcpy(info->spec.data.u.mplsoudp_encap.l2_encap.smac, &mac,
               ETH_ADDR_LEN);
        info->spec.data.u.mplsoudp_encap.ip_encap.ip_saddr =
            session_encap_mplsoudp.get_ipv4_sa();
        info->spec.data.u.mplsoudp_encap.ip_encap.ip_daddr =
            session_encap_mplsoudp.get_ipv4_da();
	info->spec.data.u.mplsoudp_encap.udp_encap.udp_sport =
            session_encap_mplsoudp.get_udp_sport();
	info->spec.data.u.mplsoudp_encap.udp_encap.udp_dport =
            session_encap_mplsoudp.get_udp_dport();
        info->spec.data.u.mplsoudp_encap.mpls1_label =
            session_encap_mplsoudp.get_mpls_label1();
        info->spec.data.u.mplsoudp_encap.mpls2_label =
            session_encap_mplsoudp.get_mpls_label2();
        info->spec.data.u.mplsoudp_encap.mpls3_label =
            session_encap_mplsoudp.get_mpls_label3();
        info->spec.data.encap_type = ENCAP_TYPE_MPLSOUDP;

    } else if (action_id ==
        SESSION_REWRITE_ENCAP_SESSION_REWRITE_ENCAP_GENEVE_ID) {
        session_rewrite_encap_geneve_entry_t session_encap_geneve;

        session_encap_geneve.clear();
        session_encap_geneve.copy_data(&session_encap_l2);

        if (!session_encap_geneve.get_valid_flag()) {
            PDS_TRACE_ERR("Invalid entry in session encap table at index %u",
                          session_rewrite_id);
            return SDK_RET_ERR;
        }
        info->spec.data.u.geneve_encap.l2_encap.insert_vlan_tag =
            session_encap_geneve.get_add_vlan_tag_flag();
        info->spec.data.u.geneve_encap.l2_encap.vlan_id =
            session_encap_geneve.get_vlan();
        mac = session_encap_geneve.get_dmac();
        memcpy(info->spec.data.u.geneve_encap.l2_encap.dmac, &mac,
               ETH_ADDR_LEN);
        mac = session_encap_geneve.get_smac(); 
        memcpy(info->spec.data.u.geneve_encap.l2_encap.smac, &mac,
               ETH_ADDR_LEN);
        info->spec.data.u.geneve_encap.ip_encap.ip_saddr =
            session_encap_geneve.get_ipv4_sa();
        info->spec.data.u.geneve_encap.ip_encap.ip_daddr =
            session_encap_geneve.get_ipv4_da();
	info->spec.data.u.geneve_encap.udp_encap.udp_sport =
            session_encap_geneve.get_udp_sport();
	info->spec.data.u.geneve_encap.udp_encap.udp_dport =
            session_encap_geneve.get_udp_dport();
        info->spec.data.u.geneve_encap.vni =
            session_encap_geneve.get_vni();
        info->spec.data.u.geneve_encap.source_slot_id =
            session_encap_geneve.get_source_slot_id();
        info->spec.data.u.geneve_encap.destination_slot_id =
            session_encap_geneve.get_destination_slot_id();
        info->spec.data.u.geneve_encap.sg_id1 =
            session_encap_geneve.get_sg_id1();
        info->spec.data.u.geneve_encap.sg_id2 =
            session_encap_geneve.get_sg_id2();
        info->spec.data.u.geneve_encap.sg_id3 =
            session_encap_geneve.get_sg_id3();
        info->spec.data.u.geneve_encap.sg_id4 =
            session_encap_geneve.get_sg_id4();
        info->spec.data.u.geneve_encap.sg_id5 =
            session_encap_geneve.get_sg_id5();
        info->spec.data.u.geneve_encap.sg_id6 =
            session_encap_geneve.get_sg_id6();
        info->spec.data.u.geneve_encap.originator_physical_ip =
            session_encap_geneve.get_originator_physical_ip();
        info->spec.data.encap_type = ENCAP_TYPE_GENEVE;
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
