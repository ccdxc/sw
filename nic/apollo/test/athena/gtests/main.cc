//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the main for the athena_app daemon
///
//----------------------------------------------------------------------------

#include <unistd.h>
#include <getopt.h>
#include <limits.h>
#include <string>
#include <iostream>
#include <gtest/gtest.h>
#include <stdarg.h>
#include <sys/stat.h>
#include "nic/sdk/asic/port.hpp"
#include "nic/apollo/api/include/pds.hpp"
#include "nic/apollo/api/include/pds_init.hpp"
#include "nic/sdk/include/sdk/base.hpp"
#include "nic/apollo/api/include/pds_batch.hpp"
#include "nic/apollo/api/include/pds_device.hpp"
#include "nic/sdk/model_sim/include/lib_model_client.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include "nic/sdk/lib/p4/p4_api.hpp"
#include "gen/p4gen/p4/include/p4pd.h"
#include "nic/apollo/p4/include/athena_defines.h"
#include "nic/apollo/p4/include/athena_table_sizes.h"
#include "trace.hpp"
#include "nic/sdk/lib/utils/utils.hpp"
#include "gen/p4gen/p4/include/ftl_table.hpp"
#include "nic/apollo/api/include/athena/pds_init.h"
#include "nic/apollo/api/include/athena/pds_vnic.h"
#include "nic/apollo/api/include/athena/pds_flow_cache.h"
#include "nic/apollo/api/include/athena/pds_flow_session_info.h"
#include "nic/apollo/api/include/athena/pds_flow_session_rewrite.h"
#include "nic/apollo/api/include/athena/pds_dnat.h"
#include "athena_gtest.hpp"

namespace core {
// number of trace files to keep
#define TRACE_NUM_FILES                        5
#define TRACE_FILE_SIZE                        (20 << 20)
static inline string
log_file (const char *logdir, const char *logfile)
{
    struct stat st = { 0 };

    if (!logdir) {
        return std::string(logfile);
    }

    // check if this log dir exists
    if (stat(logdir, &st) == -1) {
        // doesn't exist, try to create
        if (mkdir(logdir, 0755) < 0) {
            fprintf(stderr,
                    "Log directory %s/ doesn't exist, failed to create "
                    "one\n", logdir);
            return std::string("");
        }
    } else {
        // log dir exists, check if we have write permissions
        if (access(logdir, W_OK) < 0) {
            // don't have permissions to create this directory
            fprintf(stderr,
                    "No permissions to create log file in %s/\n",
                    logdir);
            return std::string("");
        }
    }
    return logdir + std::string(logfile);
}

//------------------------------------------------------------------------------
// initialize the logger
//------------------------------------------------------------------------------
static inline sdk_ret_t
logger_init (void)
{
    std::string logfile, err_logfile;

    logfile = log_file(std::getenv("LOG_DIR"), "./pds-athena-agent.log");
    err_logfile = log_file(std::getenv("PERSISTENT_LOG_DIR"), "/obfl.log");

    if (logfile.empty() || err_logfile.empty()) {
        return SDK_RET_ERR;
    }

    // initialize the logger
    core::trace_init("agent", 0x1, true, err_logfile.c_str(), logfile.c_str(),
                     TRACE_FILE_SIZE, TRACE_NUM_FILES, utils::trace_verbose);

    return SDK_RET_OK;
}

//------------------------------------------------------------------------------
// logger callback passed to SDK and PDS lib
//------------------------------------------------------------------------------
static int
sdk_logger (uint32_t mod_id, sdk_trace_level_e tracel_level,
            const char *format, ...)
{
    char       logbuf[1024];
    va_list    args;

    va_start(args, format);
    vsnprintf(logbuf, sizeof(logbuf), format, args);
    switch (tracel_level) {
    case sdk::lib::SDK_TRACE_LEVEL_ERR:
        PDS_TRACE_ERR_NO_META("{}", logbuf);
        break;
    case sdk::lib::SDK_TRACE_LEVEL_WARN:
        PDS_TRACE_WARN_NO_META("{}", logbuf);
        break;
    case sdk::lib::SDK_TRACE_LEVEL_INFO:
        PDS_TRACE_INFO_NO_META("{}", logbuf);
        break;
    case sdk::lib::SDK_TRACE_LEVEL_DEBUG:
        PDS_TRACE_DEBUG_NO_META("{}", logbuf);
        break;
    case sdk::lib::SDK_TRACE_LEVEL_VERBOSE:
        PDS_TRACE_VERBOSE_NO_META("{}", logbuf);
        break;
    default:
        break;
    }
    va_end(args);

    return 0;
}
} // namespace core

pds_ret_t
flow_table_init(void)
{
    return pds_thread_init(1);
}

void dump_pkt(std::vector<uint8_t> &pkt)
{
    for (std::vector<uint8_t>::iterator it = pkt.begin() ; it != pkt.end(); ++it) {
        printf("%02x", *it);
    }
    printf("\n");
}



sdk_ret_t send_packet(const char *out_pkt_descr, uint8_t *out_pkt, uint16_t out_pkt_len, uint32_t out_port,
        uint8_t *in_pkt, uint16_t in_pkt_len, uint32_t in_port)
{
    uint32_t                port;
    uint32_t                cos = 0;
    std::vector<uint8_t>    ipkt;
    std::vector<uint8_t>    opkt;
    std::vector<uint8_t>    epkt;

    if (out_pkt_descr)
        printf("Test with Pkt:%s\n", out_pkt_descr);

    opkt.resize(out_pkt_len);
    memcpy(opkt.data(), out_pkt, out_pkt_len);

    printf("Sending Packet of len %d B on port: %d\n", out_pkt_len, out_port);
     
    //dump_pkt(ipkt);

    step_network_pkt(opkt, out_port);

    get_next_pkt(ipkt, port, cos);

    printf("Received Packet of len %lu B on port: %d\n", ipkt.size(), port);
    //dump_pkt(opkt);

    if (in_pkt && in_pkt_len) {
        epkt.resize(in_pkt_len);
        memcpy(epkt.data(), in_pkt, in_pkt_len);
        if (port != in_port) {
            printf("Input port (%u) does not match the expected port (%u)\n", port, in_port);
            printf("Sent Packet:\n");
            dump_pkt(opkt);
            printf("Expected Packet:\n");
            dump_pkt(epkt);
            printf("Received Packet:\n");
            dump_pkt(ipkt);
            return SDK_RET_ERR;
        }
        if (in_pkt_len != ipkt.size()) {
            printf("Output packet size (%lu B) does not match the expected packet size (%u B)\n",
                    ipkt.size(), in_pkt_len);
            printf("Sent Packet:\n");
            dump_pkt(opkt);
            printf("Expected Packet:\n");
            dump_pkt(epkt);
            printf("Received Packet:\n");
            dump_pkt(ipkt);
            return SDK_RET_ERR;
        }

        if (ipkt != epkt) {
            printf("Output packet does not match the expected packet\n");
            printf("Sent Packet:\n");
            dump_pkt(opkt);
            printf("Expected Packet:\n");
            dump_pkt(epkt);
            printf("Received Packet:\n");
            dump_pkt(ipkt);
            return SDK_RET_ERR;
        }
    }
    return SDK_RET_OK;
}

sdk_ret_t send_packet_wmask(const char *out_pkt_descr, uint8_t *out_pkt, uint16_t out_pkt_len, uint32_t out_port,
			    uint8_t *in_pkt, uint16_t in_pkt_len, uint32_t in_port, uint32_t mask_start_pos)
{
    uint32_t                port;
    uint32_t                cos = 0;
    std::vector<uint8_t>    ipkt;
    std::vector<uint8_t>    opkt;
    std::vector<uint8_t>    epkt;

    if (out_pkt_descr)
        printf("Test with Pkt:%s\n", out_pkt_descr);

    opkt.resize(out_pkt_len);
    memcpy(opkt.data(), out_pkt, out_pkt_len);

    printf("Sending Packet of len %d B on port: %d\n", out_pkt_len, out_port);
     
    //dump_pkt(ipkt);

    step_network_pkt(opkt, out_port);

    get_next_pkt(ipkt, port, cos);

    printf("Received Packet of len %lu B on port: %d\n", ipkt.size(), port);
    //dump_pkt(opkt);
    //mask 2 bytes of received packet starting from mask_start_pos
    if(ipkt.size() >= (mask_start_pos + 2)) {
	for(int i=0; i<2; i++) {
	  printf("Masking byte 0x(%02x) at position (%u)\n", ipkt[i+mask_start_pos], (i+mask_start_pos));
	  ipkt[i+mask_start_pos] = 0x00;
	}
      }

    if (in_pkt && in_pkt_len) {
        epkt.resize(in_pkt_len);
        memcpy(epkt.data(), in_pkt, in_pkt_len);
        if (port != in_port) {
            printf("Input port (%u) does not match the expected port (%u)\n", port, in_port);
            printf("Sent Packet:\n");
            dump_pkt(opkt);
            printf("Expected Packet:\n");
            dump_pkt(epkt);
            printf("Received Packet:\n");
            dump_pkt(ipkt);
            return SDK_RET_ERR;
        }
        if (in_pkt_len != ipkt.size()) {
            printf("Output packet size (%lu B) does not match the expected packet size (%u B)\n",
                    ipkt.size(), in_pkt_len);
            printf("Sent Packet:\n");
            dump_pkt(opkt);
            printf("Expected Packet:\n");
            dump_pkt(epkt);
            printf("Received Packet:\n");
            dump_pkt(ipkt);
            return SDK_RET_ERR;
        }

        if (ipkt != epkt) {
            printf("Output packet does not match the expected packet\n");
            printf("Sent Packet:\n");
            dump_pkt(opkt);
            printf("Expected Packet:\n");
            dump_pkt(epkt);
            printf("Received Packet:\n");
            dump_pkt(ipkt);
            return SDK_RET_ERR;
        }
    }
    return SDK_RET_OK;
}


uint8_t     g_h_port = UPLINK_HOST;
uint8_t     g_s_port = UPLINK_SWITCH;

/*
 * Simple/Primitive resource allocation
 */
uint32_t    g_session_index = 1;
uint32_t    g_session_rewrite_index = 1;
uint32_t    g_epoch_index = 1;


/*
 * Common Infrastructure
 */

sdk_ret_t
create_s2h_session_rewrite(uint32_t session_rewrite_id,
        mac_addr_t *ep_dmac, mac_addr_t *ep_smac, uint16_t vnic_vlan)
{ 
    sdk_ret_t                                   ret = SDK_RET_OK;
    pds_flow_session_rewrite_spec_t             spec;

    memset(&spec, 0, sizeof(spec));
    spec.key.session_rewrite_id = session_rewrite_id;

    spec.data.strip_encap_header = TRUE;
    spec.data.strip_l2_header = TRUE;
    spec.data.strip_vlan_tag = TRUE;

    spec.data.nat_info.nat_type = REWRITE_NAT_TYPE_NONE;

    spec.data.encap_type = ENCAP_TYPE_L2;
    sdk::lib::memrev(spec.data.u.l2_encap.dmac, (uint8_t*)ep_dmac, sizeof(mac_addr_t));
    sdk::lib::memrev(spec.data.u.l2_encap.smac, (uint8_t*)ep_smac, sizeof(mac_addr_t));
    spec.data.u.l2_encap.insert_vlan_tag = TRUE;
    spec.data.u.l2_encap.vlan_id = vnic_vlan;

    ret = (sdk_ret_t)pds_flow_session_rewrite_create(&spec);
    if (ret != SDK_RET_OK) {
        printf("Failed to program session rewrite s2h info : %u\n", ret);
    }

    return ret;
}

sdk_ret_t
create_s2h_session_rewrite_nat_ipv4(uint32_t session_rewrite_id,
        mac_addr_t *ep_dmac, mac_addr_t *ep_smac, uint16_t vnic_vlan,
        pds_flow_session_rewrite_nat_type_t nat_type,
        ipv4_addr_t ipv4_addr)
{ 
    pds_ret_t                                   ret = PDS_RET_OK;
    pds_flow_session_rewrite_spec_t             spec;

    memset(&spec, 0, sizeof(spec));
    spec.key.session_rewrite_id = session_rewrite_id;

    spec.data.strip_encap_header = TRUE;
    spec.data.strip_l2_header = TRUE;
    spec.data.strip_vlan_tag = TRUE;

    spec.data.nat_info.nat_type = nat_type;
    spec.data.nat_info.u.ipv4_addr = (uint32_t) ipv4_addr;

    spec.data.encap_type = ENCAP_TYPE_L2;
    sdk::lib::memrev(spec.data.u.l2_encap.dmac, (uint8_t*)ep_dmac, sizeof(mac_addr_t));
    sdk::lib::memrev(spec.data.u.l2_encap.smac, (uint8_t*)ep_smac, sizeof(mac_addr_t));
    spec.data.u.l2_encap.insert_vlan_tag = TRUE;
    spec.data.u.l2_encap.vlan_id = vnic_vlan;

    ret = pds_flow_session_rewrite_create(&spec);
    if (ret != PDS_RET_OK) {
        printf("Failed to program session rewrite s2h info : %u\n", ret);
    }

    return (sdk_ret_t) ret;
}

sdk_ret_t
create_s2h_session_rewrite_nat_ipv6(uint32_t session_rewrite_id,
        mac_addr_t *ep_dmac, mac_addr_t *ep_smac, uint16_t vnic_vlan,
        pds_flow_session_rewrite_nat_type_t nat_type,
        ipv6_addr_t *ipv6_addr)
{ 
    pds_ret_t                                   ret = PDS_RET_OK;
    pds_flow_session_rewrite_spec_t             spec;

    memset(&spec, 0, sizeof(spec));
    spec.key.session_rewrite_id = session_rewrite_id;

    spec.data.strip_encap_header = TRUE;
    spec.data.strip_l2_header = TRUE;
    spec.data.strip_vlan_tag = TRUE;

    spec.data.nat_info.nat_type = nat_type;
    sdk::lib::memrev(spec.data.nat_info.u.ipv6_addr, (uint8_t *)ipv6_addr,
            sizeof(ipv6_addr_t));

    spec.data.encap_type = ENCAP_TYPE_L2;
    sdk::lib::memrev(spec.data.u.l2_encap.dmac, (uint8_t*)ep_dmac, sizeof(mac_addr_t));
    sdk::lib::memrev(spec.data.u.l2_encap.smac, (uint8_t*)ep_smac, sizeof(mac_addr_t));
    spec.data.u.l2_encap.insert_vlan_tag = TRUE;
    spec.data.u.l2_encap.vlan_id = vnic_vlan;

    ret = pds_flow_session_rewrite_create(&spec);
    if (ret != PDS_RET_OK) {
        printf("Failed to program session rewrite s2h info : %u\n", ret);
    }

    return (sdk_ret_t) ret;
}

sdk_ret_t
create_h2s_session_rewrite_mplsoudp(uint32_t session_rewrite_id,
        mac_addr_t *substrate_dmac, mac_addr_t *substrate_smac,
        uint16_t substrate_vlan,
        uint32_t substrate_sip, uint32_t substrate_dip,
        uint32_t mpls1_label, uint32_t mpls2_label)
{ 
    pds_ret_t                                   ret = PDS_RET_OK;
    pds_flow_session_rewrite_spec_t             spec;

    memset(&spec, 0, sizeof(spec));
    spec.key.session_rewrite_id = session_rewrite_id;

    spec.data.strip_l2_header = TRUE;
    spec.data.strip_vlan_tag = TRUE;

    spec.data.nat_info.nat_type = REWRITE_NAT_TYPE_NONE;

    spec.data.encap_type = ENCAP_TYPE_MPLSOUDP;
    sdk::lib::memrev(spec.data.u.mplsoudp_encap.l2_encap.dmac, (uint8_t*)substrate_dmac, sizeof(mac_addr_t));
    sdk::lib::memrev(spec.data.u.mplsoudp_encap.l2_encap.smac, (uint8_t*)substrate_smac, sizeof(mac_addr_t));
    spec.data.u.mplsoudp_encap.l2_encap.insert_vlan_tag = TRUE;
    spec.data.u.mplsoudp_encap.l2_encap.vlan_id = substrate_vlan;

    spec.data.u.mplsoudp_encap.ip_encap.ip_saddr = substrate_sip;
    spec.data.u.mplsoudp_encap.ip_encap.ip_daddr = substrate_dip;

    spec.data.u.mplsoudp_encap.mpls1_label = mpls1_label;
    spec.data.u.mplsoudp_encap.mpls2_label = mpls2_label;

    ret = pds_flow_session_rewrite_create(&spec);
    if (ret != PDS_RET_OK) {
        printf("Failed to program session rewrite h2s info : %u\n", ret);
    }

    return (sdk_ret_t) ret;
}

sdk_ret_t
create_h2s_session_rewrite_mplsoudp_nat_ipv4(uint32_t session_rewrite_id,
        mac_addr_t *substrate_dmac, mac_addr_t *substrate_smac,
        uint16_t substrate_vlan,
        uint32_t substrate_sip, uint32_t substrate_dip,
        uint32_t mpls1_label, uint32_t mpls2_label,
        pds_flow_session_rewrite_nat_type_t nat_type,
        ipv4_addr_t ipv4_addr)
{ 
    pds_ret_t                                   ret = PDS_RET_OK;
    pds_flow_session_rewrite_spec_t             spec;

    memset(&spec, 0, sizeof(spec));
    spec.key.session_rewrite_id = session_rewrite_id;

    spec.data.strip_l2_header = TRUE;
    spec.data.strip_vlan_tag = TRUE;

    spec.data.nat_info.nat_type = nat_type;
    spec.data.nat_info.u.ipv4_addr = (uint32_t) ipv4_addr;

    spec.data.encap_type = ENCAP_TYPE_MPLSOUDP;
    sdk::lib::memrev(spec.data.u.mplsoudp_encap.l2_encap.dmac, (uint8_t*)substrate_dmac, sizeof(mac_addr_t));
    sdk::lib::memrev(spec.data.u.mplsoudp_encap.l2_encap.smac, (uint8_t*)substrate_smac, sizeof(mac_addr_t));
    spec.data.u.mplsoudp_encap.l2_encap.insert_vlan_tag = TRUE;
    spec.data.u.mplsoudp_encap.l2_encap.vlan_id = substrate_vlan;

    spec.data.u.mplsoudp_encap.ip_encap.ip_saddr = substrate_sip;
    spec.data.u.mplsoudp_encap.ip_encap.ip_daddr = substrate_dip;

    spec.data.u.mplsoudp_encap.mpls1_label = mpls1_label;
    spec.data.u.mplsoudp_encap.mpls2_label = mpls2_label;

    ret = pds_flow_session_rewrite_create(&spec);
    if (ret != PDS_RET_OK) {
        printf("Failed to program session rewrite h2s info : %u\n", ret);
    }

    return (sdk_ret_t) ret;
}

sdk_ret_t
create_h2s_session_rewrite_mplsoudp_nat_ipv6(uint32_t session_rewrite_id,
        mac_addr_t *substrate_dmac, mac_addr_t *substrate_smac,
        uint16_t substrate_vlan,
        uint32_t substrate_sip, uint32_t substrate_dip,
        uint32_t mpls1_label, uint32_t mpls2_label,
        pds_flow_session_rewrite_nat_type_t nat_type,
        ipv6_addr_t *ipv6_addr)
{ 
    pds_ret_t                                   ret = PDS_RET_OK;
    pds_flow_session_rewrite_spec_t             spec;

    memset(&spec, 0, sizeof(spec));
    spec.key.session_rewrite_id = session_rewrite_id;

    spec.data.strip_l2_header = TRUE;
    spec.data.strip_vlan_tag = TRUE;

    spec.data.nat_info.nat_type = nat_type;
    sdk::lib::memrev(spec.data.nat_info.u.ipv6_addr, (uint8_t *)ipv6_addr,
            sizeof(ipv6_addr_t));

    spec.data.encap_type = ENCAP_TYPE_MPLSOUDP;
    sdk::lib::memrev(spec.data.u.mplsoudp_encap.l2_encap.dmac, (uint8_t*)substrate_dmac, sizeof(mac_addr_t));
    sdk::lib::memrev(spec.data.u.mplsoudp_encap.l2_encap.smac, (uint8_t*)substrate_smac, sizeof(mac_addr_t));
    spec.data.u.mplsoudp_encap.l2_encap.insert_vlan_tag = TRUE;
    spec.data.u.mplsoudp_encap.l2_encap.vlan_id = substrate_vlan;

    spec.data.u.mplsoudp_encap.ip_encap.ip_saddr = substrate_sip;
    spec.data.u.mplsoudp_encap.ip_encap.ip_daddr = substrate_dip;

    spec.data.u.mplsoudp_encap.mpls1_label = mpls1_label;
    spec.data.u.mplsoudp_encap.mpls2_label = mpls2_label;

    ret = pds_flow_session_rewrite_create(&spec);
    if (ret != PDS_RET_OK) {
        printf("Failed to program session rewrite h2s info : %u\n", ret);
    }

    return (sdk_ret_t)ret;
}

sdk_ret_t
create_session_info_all(uint32_t session_id, uint32_t conntrack_id,
                uint8_t skip_flow_log, mac_addr_t *host_mac,
                uint16_t h2s_epoch_vnic, uint32_t h2s_epoch_vnic_id,
                uint16_t h2s_epoch_mapping, uint32_t h2s_epoch_mapping_id,
                uint16_t h2s_policer_bw1_id, uint16_t h2s_policer_bw2_id,
                uint16_t h2s_vnic_stats_id, uint8_t *h2s_vnic_stats_mask,
                uint16_t h2s_vnic_histogram_latency_id, uint16_t h2s_vnic_histogram_packet_len_id,
                uint8_t h2s_tcp_flags_bitmap,
                uint32_t h2s_session_rewrite_id,
                uint16_t h2s_allowed_flow_state_bitmask,
                pds_egress_action_t h2s_egress_action,

                uint16_t s2h_epoch_vnic, uint32_t s2h_epoch_vnic_id,
                uint16_t s2h_epoch_mapping, uint32_t s2h_epoch_mapping_id,
                uint16_t s2h_policer_bw1_id, uint16_t s2h_policer_bw2_id,
                uint16_t s2h_vnic_stats_id, uint8_t *s2h_vnic_stats_mask,
                uint16_t s2h_vnic_histogram_latency_id, uint16_t s2h_vnic_histogram_packet_len_id,
                uint8_t s2h_tcp_flags_bitmap,
                uint32_t s2h_session_rewrite_id,
                uint16_t s2h_allowed_flow_state_bitmask,
                pds_egress_action_t s2h_egress_action)
{
    pds_ret_t                               ret = PDS_RET_OK;
    pds_flow_session_spec_t                 spec;

    memset(&spec, 0, sizeof(spec));
    spec.key.session_info_id = session_id;
    spec.key.direction = (SWITCH_TO_HOST | HOST_TO_SWITCH);

    spec.data.conntrack_id = conntrack_id;
    spec.data.skip_flow_log = skip_flow_log;
    sdk::lib::memrev(spec.data.host_mac, (uint8_t*)host_mac, sizeof(mac_addr_t));

    /* Host-to-switch */
    spec.data.host_to_switch_flow_info.epoch_vnic = h2s_epoch_vnic;
    spec.data.host_to_switch_flow_info.epoch_vnic_id = h2s_epoch_vnic_id;
    spec.data.host_to_switch_flow_info.epoch_mapping = h2s_epoch_mapping;
    spec.data.host_to_switch_flow_info.policer_bw1_id = h2s_policer_bw1_id;
    spec.data.host_to_switch_flow_info.policer_bw2_id = h2s_policer_bw2_id;
    spec.data.host_to_switch_flow_info.vnic_stats_id = h2s_vnic_stats_id;
    sdk::lib::memrev(spec.data.host_to_switch_flow_info.vnic_stats_mask,
            h2s_vnic_stats_mask, PDS_FLOW_STATS_MASK_LEN);
    spec.data.host_to_switch_flow_info.vnic_histogram_latency_id = h2s_vnic_histogram_latency_id;
    spec.data.host_to_switch_flow_info.vnic_histogram_packet_len_id = h2s_vnic_histogram_packet_len_id;
    spec.data.host_to_switch_flow_info.tcp_flags_bitmap = h2s_tcp_flags_bitmap;
    spec.data.host_to_switch_flow_info.rewrite_id = h2s_session_rewrite_id;
    spec.data.host_to_switch_flow_info.allowed_flow_state_bitmask = h2s_allowed_flow_state_bitmask;
    spec.data.host_to_switch_flow_info.egress_action = h2s_egress_action;

    /* Switch-to-host */
    spec.data.switch_to_host_flow_info.epoch_vnic = s2h_epoch_vnic;
    spec.data.switch_to_host_flow_info.epoch_vnic_id = s2h_epoch_vnic_id;
    spec.data.switch_to_host_flow_info.epoch_mapping = s2h_epoch_mapping;
    spec.data.switch_to_host_flow_info.policer_bw1_id = s2h_policer_bw1_id;
    spec.data.switch_to_host_flow_info.policer_bw2_id = s2h_policer_bw2_id;
    spec.data.switch_to_host_flow_info.vnic_stats_id = s2h_vnic_stats_id;
    sdk::lib::memrev(spec.data.switch_to_host_flow_info.vnic_stats_mask,
            s2h_vnic_stats_mask, PDS_FLOW_STATS_MASK_LEN);
    spec.data.switch_to_host_flow_info.vnic_histogram_latency_id = s2h_vnic_histogram_latency_id;
    spec.data.switch_to_host_flow_info.vnic_histogram_packet_len_id = s2h_vnic_histogram_packet_len_id;
    spec.data.switch_to_host_flow_info.tcp_flags_bitmap = s2h_tcp_flags_bitmap;
    spec.data.switch_to_host_flow_info.rewrite_id = s2h_session_rewrite_id;
    spec.data.switch_to_host_flow_info.allowed_flow_state_bitmask = s2h_allowed_flow_state_bitmask;
    spec.data.switch_to_host_flow_info.egress_action = s2h_egress_action;

    ret = pds_flow_session_info_create(&spec);
    if (ret != PDS_RET_OK) {
        printf("Failed to program session s2h info : %u\n", ret);
    }
    return (sdk_ret_t)ret;
}
                            
sdk_ret_t
vlan_to_vnic_map(uint16_t vlan_id, uint16_t vnic_id)
{
    pds_vlan_to_vnic_map_spec_t     spec;
    pds_ret_t                       ret = PDS_RET_OK;

    spec.key.vlan_id = vlan_id;
    spec.data.vnic_type = VNIC_TYPE_L3;
    spec.data.vnic_id = vnic_id;

    ret = pds_vlan_to_vnic_map_create(&spec);
    if (ret != PDS_RET_OK) {
        printf("Failed to setup VLAN: %hu to VNIC:%hu mapping\n",
                vlan_id, vnic_id);
    }
    printf("Setup VLAN: %hu to VNIC:%hu mapping\n",
            vlan_id, vnic_id);
    return (sdk_ret_t) ret;
}


sdk_ret_t
mpls_label_to_vnic_map(uint32_t mpls_label, uint16_t vnic_id)
{
    pds_mpls_label_to_vnic_map_spec_t       spec;
    pds_ret_t                               ret = PDS_RET_OK;

    spec.key.mpls_label = mpls_label;
    spec.data.vnic_type = VNIC_TYPE_L3;
    spec.data.vnic_id = vnic_id;

    ret = pds_mpls_label_to_vnic_map_create(&spec);
    if (ret != PDS_RET_OK) {
        printf("Failed to setup MPLS Label: %u to VNIC:%hu mapping\n",
                mpls_label, vnic_id);
    }
    printf("Setup MPLS Label: %u to VNIC:%hu mapping\n",
            mpls_label, vnic_id);
    return (sdk_ret_t) ret;
}


sdk_ret_t
create_dnat_map_ipv4(uint16_t vnic_id, ipv4_addr_t v4_nat_dip, 
        ipv4_addr_t v4_orig_dip, uint16_t dnat_epoch)
{
    pds_dnat_mapping_spec_t         spec;

    memset(&spec, 0, sizeof(spec));

    spec.key.vnic_id = vnic_id;
    spec.key.key_type = IP_AF_IPV4;
    memcpy(spec.key.addr, &v4_nat_dip, sizeof(ipv4_addr_t));

    spec.data.addr_type = IP_AF_IPV4;
    memcpy(spec.data.addr, &v4_orig_dip, sizeof(ipv4_addr_t));
    spec.data.epoch = dnat_epoch;

    return (sdk_ret_t)pds_dnat_map_entry_create(&spec);
}

sdk_ret_t
create_dnat_map_ipv6(uint16_t vnic_id, ipv6_addr_t *v6_nat_dip, 
        ipv6_addr_t *v6_orig_dip, uint16_t dnat_epoch)
{
    pds_dnat_mapping_spec_t         spec;

    memset(&spec, 0, sizeof(spec));

    spec.key.vnic_id = vnic_id;
    spec.key.key_type = IP_AF_IPV6;
    sdk::lib::memrev(spec.key.addr, (uint8_t *)v6_nat_dip,
            sizeof(ipv6_addr_t));

    spec.data.addr_type = IP_AF_IPV6;
    sdk::lib::memrev(spec.data.addr, (uint8_t *)v6_orig_dip,
            sizeof(ipv6_addr_t));
    spec.data.epoch = dnat_epoch;

    return (sdk_ret_t)pds_dnat_map_entry_create(&spec);
}


sdk_ret_t
setup_flows(void)
{
    sdk_ret_t       ret = SDK_RET_OK;

    ret = athena_gtest_setup_flows_udp();
    if (ret != SDK_RET_OK) {
        return ret;
    }

    ret = athena_gtest_setup_flows_tcp();
    if (ret != SDK_RET_OK) {
        return ret;
    }

    ret = athena_gtest_setup_flows_slowpath();
    if (ret != SDK_RET_OK) {
        return ret;
    }

    ret = athena_gtest_setup_flows_icmp();
    if (ret != SDK_RET_OK) {
        return ret;
    }

    ret = athena_gtest_setup_flows_nat();
    if (ret != SDK_RET_OK) {
        return ret;
    }

    return ret;
}

void
dump_single_flow(pds_flow_iter_cb_arg_t *iter_cb_arg)
{
    pds_flow_key_t *key = &iter_cb_arg->flow_key;
    pds_flow_data_t *data = &iter_cb_arg->flow_appdata;

    printf("SrcIP:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x "
           "DstIP:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x "
           "Dport:%u Sport:%u Proto:%u "
           "Ktype:%u VNICID:%u "
           "index:%u index_type:%u\n\n",
           key->ip_saddr[0], key->ip_saddr[1], key->ip_saddr[2], key->ip_saddr[3],
           key->ip_saddr[4], key->ip_saddr[5], key->ip_saddr[6], key->ip_saddr[7],
           key->ip_saddr[8], key->ip_saddr[9], key->ip_saddr[10], key->ip_saddr[11],
           key->ip_saddr[12], key->ip_saddr[13], key->ip_saddr[14], key->ip_saddr[15],
           key->ip_daddr[0], key->ip_daddr[1], key->ip_daddr[2], key->ip_daddr[3],
           key->ip_daddr[4], key->ip_daddr[5], key->ip_daddr[6], key->ip_daddr[7],
           key->ip_daddr[8], key->ip_daddr[9], key->ip_daddr[10], key->ip_daddr[11],
           key->ip_daddr[12], key->ip_daddr[13], key->ip_daddr[14], key->ip_daddr[15],
           key->l4.tcp_udp.dport, key->l4.tcp_udp.sport,
           key->ip_proto, (uint8_t)key->key_type, key->vnic_id,
           data->index, (uint8_t)data->index_type);
    return;
}

void
iterate_dump_flows(void)
{
    pds_flow_iter_cb_arg_t iter_cb_arg = { 0 };

    ASSERT_TRUE(pds_flow_cache_entry_iterate(dump_single_flow, &iter_cb_arg)
        == PDS_RET_OK);
}

void
dump_stats(pds_flow_stats_t *stats)
{
    printf("\nPrinting Flow cache statistics\n");
    printf("Insert %lu, Insert_fail_dupl %lu, Insert_fail %lu, "
           "Insert_fail_recirc %lu\n"
           "Remove %lu, Remove_not_found %lu, Remove_fail %lu\n"
           "Update %lu, Update_fail %lu\n"
           "Get %lu, Get_fail %lu\n"
           "Reserve %lu, reserve_fail %lu\n"
           "Release %lu, Release_fail %lu\n"
           "Tbl_entries %lu, Tbl_collision %lu\n"
           "Tbl_insert %lu, Tbl_remove %lu, Tbl_read %lu, Tbl_write %lu\n",
           stats->api_insert,
           stats->api_insert_duplicate,
           stats->api_insert_fail,
           stats->api_insert_recirc_fail,
           stats->api_remove,
           stats->api_remove_not_found,
           stats->api_remove_fail,
           stats->api_update,
           stats->api_update_fail,
           stats->api_get,
           stats->api_get_fail,
           stats->api_reserve,
           stats->api_reserve_fail,
           stats->api_release,
           stats->api_release_fail,
           stats->table_entries, stats->table_collisions,
           stats->table_insert, stats->table_remove,
           stats->table_read, stats->table_write);
    for (int i= 0; i < PDS_FLOW_TABLE_MAX_RECIRC; i++) {
         printf("Tbl_lvl %u, Tbl_insert %lu, Tbl_remove %lu\n",
                 i, stats->table_insert_lvl[i], stats->table_remove_lvl[i]);
    }
    return;
}

void
print_stats(void)
{
    pds_flow_stats_t stats = { 0 };

    ASSERT_TRUE(pds_flow_cache_stats_get(1, &stats) == PDS_RET_OK);
    dump_stats(&stats);
}

TEST(athena_gtest, sim)
{
    sdk_ret_t           ret = SDK_RET_OK;

    /* Setup all flows */
    ASSERT_TRUE((ret = setup_flows()) == SDK_RET_OK);

    /* UDP Flow tests */
    ASSERT_TRUE(athena_gtest_test_flows_udp() == SDK_RET_OK);

    /* TCP Flow tests */
    ASSERT_TRUE(athena_gtest_test_flows_tcp() == SDK_RET_OK);

    /* Slowpath Flow tests */
    ASSERT_TRUE(athena_gtest_test_flows_slowpath() == SDK_RET_OK);

    /* ICMP Flow tests */
    ASSERT_TRUE(athena_gtest_test_flows_icmp() == SDK_RET_OK);

    /* NAT Flow tests */
    ASSERT_TRUE(athena_gtest_test_flows_nat() == SDK_RET_OK);

    iterate_dump_flows();

    print_stats();

    pds_global_teardown();

}


void inline
print_usage (char **argv)
{
    fprintf(stdout, "Usage : %s -c|--config <cfg.json>\n", argv[0]);
}


int
main (int argc, char **argv)
{
    int          oc;
    string       cfg_path, cfg_file, profile, pipeline, file;
    boost::property_tree::ptree pt;
    pds_ret_t ret;

    struct option longopts[] = {
       { "config",    required_argument, NULL, 'c' },
       { "profile",   required_argument, NULL, 'p' },
       { "feature",   required_argument, NULL, 'f' },
       { "help",      no_argument,       NULL, 'h' },
       { 0,           0,                 0,     0 }
    };

    // parse CLI options
    while ((oc = getopt_long(argc, argv, ":hc:p:f:W;", longopts, NULL)) != -1) {
        switch (oc) {
        case 'c':
            if (optarg) {
                cfg_file = std::string(optarg);
            } else {
                fprintf(stderr, "config file is not specified\n");
                print_usage(argv);
                exit(1);
            }
            break;

        case 'p':
            if (optarg) {
                profile = std::string(optarg);
            } else {
                fprintf(stderr, "profile is not specified\n");
                print_usage(argv);
                exit(1);
            }
            break;


        case 'h':
            print_usage(argv);
            exit(0);
            break;

        case ':':
            fprintf(stderr, "%s: option -%c requires an argument\n",
                    argv[0], optopt);
            print_usage(argv);
            exit(1);
            break;

        case '?':
        default:
            fprintf(stderr, "%s: option -%c is invalid, quitting ...\n",
                    argv[0], optopt);
            print_usage(argv);
            exit(1);
            break;
        }
    }

    // form the full path to the config directory
    cfg_path = std::string(std::getenv("CONFIG_PATH"));
    if (cfg_path.empty()) {
        cfg_path = std::string("./");
    } else {
        cfg_path += "/";
    }

    // read pipeline.json file to figure out pipeline
    file = cfg_path + "/pipeline.json";
    if (access(file.c_str(), R_OK) < 0) {
        fprintf(stderr, "pipeline.json doesn't exist or not accessible\n");
        exit(1);
    }

    // parse pipeline.json to determine pipeline
    try {
        std::ifstream json_cfg(file.c_str());
        read_json(json_cfg, pt);
        pipeline = pt.get<std::string>("pipeline");
    } catch (...) {
        fprintf(stderr, "pipeline.json doesn't have pipeline field\n");
        exit(1);
    }
    if  (pipeline.compare("athena") != 0) {
        fprintf(stderr, "Unknown pipeline %s\n", pipeline.c_str());
        exit(1);
    }

   // make sure the cfg file exists
    file = cfg_path + "/" + pipeline + "/" + std::string(cfg_file);
    if (access(file.c_str(), R_OK) < 0) {
        fprintf(stderr, "Config file %s doesn't exist or not accessible\n",
                file.c_str());
        exit(1);
    }

    // Initialize the PDS functionality
    pds_cinit_params_t init_params;

    memset(&init_params, 0, sizeof(init_params));
    init_params.init_mode = PDS_CINIT_MODE_COLD_START;
    init_params.trace_cb  = (void*) core::sdk_logger;
/*
    init_params.pipeline  = pipeline;
    init_params.cfg_file  = cfg_file;
    init_params.memory_profile = PDS_MEMORY_PROFILE_DEFAULT;
    init_params.scale_profile = PDS_SCALE_PROFILE_DEFAULT;
    if (!profile.empty()) {
        if (profile.compare("p1") == 0) {
            init_params.scale_profile = PDS_SCALE_PROFILE_P1;
        } else if (profile.compare("p2") == 0) {
            init_params.scale_profile = PDS_SCALE_PROFILE_P2;
        }
    }
*/
    // initialize the logger instance
    core::logger_init();

    ret = pds_global_init(&init_params);
    if (ret != PDS_RET_OK) {
        fprintf(stderr, "PDS global init failed with ret %u\n", ret);
        exit(1);
    }

    flow_table_init();

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

